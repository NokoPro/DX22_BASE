/**
 * @file World.h
 * @brief ECSにおけるWorldクラス定義
 * @details
 * Entity(エンティティ)の生成・破棄、Component(コンポーネント)の管理、
 * およびView(システム処理用の走査)を担うクラスです。
 *
 * 依存関係：
 *   - ECS.h（EntityId, IComponentStorage, ComponentTypeの定義を含む）
 */
#pragma once
#include "ECS.h"
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <memory>
#include <cassert>
#include <utility>

 /**
  * @brief 各型T専用のコンポーネントストレージ
  * @tparam T 管理対象のコンポーネント型
  */
template<class T>
struct ComponentStorage final : IComponentStorage {
    /// エンティティIDをキーとするコンポーネントの連想配列
    std::unordered_map<EntityId, T> data;

    /// 指定エンティティのコンポーネントを削除
    void Remove(EntityId e) override { data.erase(e); }

    /// 指定エンティティがコンポーネントを持つか確認
    bool Has(EntityId e) const override { return data.find(e) != data.end(); }
};

/**
 * @class World
 * @brief ECS全体を管理する中心クラス
 * @details
 * 各コンポーネント型ごとのストレージを保持し、
 * エンティティの生成・削除、コンポーネントの追加/取得/削除、
 * そして複数コンポーネントを組み合わせて走査(View)を行う機能を提供します。
 */
class World {
public:
    /// @brief デフォルトコンストラクタ
    World() = default;

    //--------------------------------------------------------------------------
    // エンティティ管理
    //--------------------------------------------------------------------------

    /**
     * @brief 新しいエンティティを生成
     * @return 生成されたエンティティID
     */
    EntityId Create() {
        if (m_next == kInvalidEntity) m_next = 1;
        return m_next++;
    }

    /**
     * @brief 指定エンティティを破棄
     * @param e 削除するエンティティID
     * @details 登録されている全てのストレージから対象のエンティティを削除します。
     */
    void Destroy(EntityId e) {
        for (auto& kv : m_storages) {
            kv.second->Remove(e);
        }
    }

    //--------------------------------------------------------------------------
    // コンポーネント操作
    //--------------------------------------------------------------------------

    /**
     * @brief コンポーネントを追加または上書き
     * @tparam T 追加するコンポーネント型
     * @param e 対象エンティティID
     * @param args コンストラクタ引数
     * @return 追加または上書きされたコンポーネント参照
     */
    template<class T, class... Args>
    T& Add(EntityId e, Args&&... args) {
        auto& s = storage<T>();
        auto it = s.find(e);
        if (it == s.end()) {
            it = s.emplace(e, T{ std::forward<Args>(args)... }).first;
        }
        else {
            it->second = T{ std::forward<Args>(args)... };
        }
        return it->second;
    }

    /**
     * @brief 指定エンティティがコンポーネントを保持しているか確認
     * @tparam T コンポーネント型
     * @param e エンティティID
     * @return true:保持している / false:保持していない
     */
    template<class T>
    bool Has(EntityId e) const {
        const auto* s = find<T>();
        return s && s->count(e) != 0;
    }

    /**
     * @brief コンポーネントを取得（非const版）
     * @tparam T コンポーネント型
     * @param e エンティティID
     * @return 参照
     * @note 存在しない場合はassertで停止します。
     */
    template<class T>
    T& Get(EntityId e) {
        auto* s = find<T>();
        assert(s && "Get<T>: storage not found");
        auto it = s->find(e);
        assert(it != s->end() && "Get<T>: entity doesn't have T");
        return it->second;
    }

    /**
     * @brief コンポーネントを取得（const版）
     * @tparam T コンポーネント型
     * @param e エンティティID
     * @return const参照
     */
    template<class T>
    const T& Get(EntityId e) const {
        const auto* s = find<T>();
        assert(s && "Get<T> const: storage not found");
        auto it = s->find(e);
        assert(it != s->end() && "Get<T> const: entity doesn't have T");
        return it->second;
    }

    /**
     * @brief コンポーネントを削除
     * @tparam T コンポーネント型
     * @param e 対象エンティティID
     */
    template<class T>
    void Remove(EntityId e) {
        auto* s = find<T>();
        if (s) s->erase(e);
    }

    //--------------------------------------------------------------------------
    // View処理（ラムダで複数コンポーネントを走査）
    //--------------------------------------------------------------------------

    /**
     * @brief 非constなView走査
     * @tparam A, Rest... 対象コンポーネント型群
     * @tparam Fn 呼び出す関数型
     * @param fn 呼び出すラムダ（例: [](EntityId, A&, Rest&...){}）
     * @details
     * 指定した全てのコンポーネントを持つエンティティを走査し、関数を呼び出します。
     */
    template<class A, class... Rest, class Fn>
    void View(Fn&& fn) {
        auto* sa = find<A>();
        if (!sa) return;
        for (auto& kv : *sa) {
            const EntityId e = kv.first;
            if (has_all_rest<Rest...>(e)) {
                fn(e, Get<A>(e), Get<Rest>(e)...);
            }
        }
    }

    /**
     * @brief constなView走査
     * @tparam A, Rest... 対象コンポーネント型群
     * @tparam Fn 呼び出す関数型
     * @param fn 呼び出すラムダ（例: [](EntityId, const A&, const Rest&...){}）
     */
    template<class A, class... Rest, class Fn>
    void View(Fn&& fn) const {
        auto const* sa = find<A>();
        if (!sa) return;
        for (auto const& kv : *sa) {
            const EntityId e = kv.first;
            if (has_all_rest<Rest...>(e)) {
                fn(e, Get<const A>(e), Get<const Rest>(e)...);
            }
        }
    }

private:
    //--------------------------------------------------------------------------
    // 内部ヘルパ関数群
    //--------------------------------------------------------------------------

    /**
     * @brief 指定型のストレージを取得（存在しなければ生成）
     * @tparam T コンポーネント型
     * @return ストレージへの参照
     */
    template<class T>
    std::unordered_map<EntityId, T>& storage() {
        ComponentType key = std::type_index(typeid(T));
        auto it = m_storages.find(key);
        if (it == m_storages.end()) {
            it = m_storages.emplace(key, std::unique_ptr<IComponentStorage>(new ComponentStorage<T>())).first;
        }
        return static_cast<ComponentStorage<T>*>(it->second.get())->data;
    }

    /**
     * @brief 指定型ストレージを検索（非const版）
     * @tparam T コンポーネント型
     * @return 見つかればポインタ、なければnullptr
     */
    template<class T>
    std::unordered_map<EntityId, T>* find() {
        ComponentType key = std::type_index(typeid(T));
        auto it = m_storages.find(key);
        if (it == m_storages.end()) return nullptr;
        return &static_cast<ComponentStorage<T>*>(it->second.get())->data;
    }

    /**
     * @brief 指定型ストレージを検索（const版）
     * @tparam T コンポーネント型
     * @return 見つかればポインタ、なければnullptr
     */
    template<class T>
    const std::unordered_map<EntityId, T>* find() const {
        ComponentType key = std::type_index(typeid(T));
        auto it = m_storages.find(key);
        if (it == m_storages.end()) return nullptr;
        return &static_cast<const ComponentStorage<T>*>(it->second.get())->data;
    }

    /**
     * @brief 可変長パックの "空" ケースを受ける末端版
     * @return 常にtrue
     */
    template<class... Ts>
    typename std::enable_if<sizeof...(Ts) == 0, bool>::type
        has_all_rest(EntityId) const {
        return true;
    }

    /**
     * @brief コンポーネントを全て保持しているかを再帰的に確認
     * @tparam T1, Ts... 対象コンポーネント型群
     * @param e エンティティID
     * @return 全て保持していればtrue
     */
    template<class T1, class... Ts>
    bool has_all_rest(EntityId e) const {
        return Has<T1>(e) && has_all_rest<Ts...>(e);
    }

private:
    /// 各コンポーネント型に対応するストレージマップ
    std::unordered_map<ComponentType, std::unique_ptr<IComponentStorage>> m_storages;

    /// 次に割り当てるエンティティID
    EntityId m_next = kInvalidEntity;
};
