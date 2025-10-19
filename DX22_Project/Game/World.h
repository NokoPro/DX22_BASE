#pragma once
#include "ECS.h"
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <cassert>
#include <memory>      // ★ 追加：unique_ptr
#include <typeindex>   // 念のため（ComponentTypeの実装次第で不要なら外してOK）

/**
 * @file World.h
 * @brief Entity と Component の生成・管理を行う中心クラス（C++14対応）
 * @details
 * - Add<T>(entity, args...) / Get<T>(entity) / Has<T>(entity)
 * - View<A,B>(lambda) で2種コンポーネントを持つエンティティへアクセス
 * - C++14対応のため構造化束縛は使用していません
 */
class World
{
public:
    World() = default;
    ~World() = default;

    //-------------------------------------------------------------------------
    // ■ Entity
    //-------------------------------------------------------------------------

    /// @brief 新しいエンティティを生成
    EntityId Create()
    {
        return ++m_nextEntity;
    }

    /// @brief エンティティを削除（関連コンポーネントも削除）
    void Destroy(EntityId e)
    {
        // C++14 なので構造化束縛は使わない
        for (auto it = m_components.begin(); it != m_components.end(); ++it)
        {
            it->second->Remove(e);
        }
    }

    //-------------------------------------------------------------------------
    // ■ Component 操作
    //-------------------------------------------------------------------------

    /// @brief コンポーネントを追加（存在すれば上書き）
    template<class T, class... Args>
    T& Add(EntityId e, Args&&... args)
    {
        auto& store = GetOrCreateStorage<T>();
        return store[e] = T{ std::forward<Args>(args)... };
    }

    /// @brief コンポーネントを取得（存在しなければ nullptr）
    template<class T>
    T* Get(EntityId e)
    {
        auto* s = FindStorage<T>();
        if (!s) return nullptr;
        auto it = s->find(e);
        return (it == s->end()) ? nullptr : &it->second;
    }

    /// @brief コンポーネントを削除
    template<class T>
    void Remove(EntityId e)
    {
        auto* s = FindStorage<T>();
        if (s) s->erase(e);
    }

    /// @brief コンポーネントの有無を確認
    template<class T>
    bool Has(EntityId e) const
    {
        auto* s = FindStorage<T>();
        if (!s) return false;
        return s->count(e) != 0;
    }

    //-------------------------------------------------------------------------
    // ■ View
    //-------------------------------------------------------------------------

    /// @brief 特定のコンポーネント型1種を持つエンティティを走査
    template<class A, class Fn>
    void View(Fn&& fn)
    {
        auto* a = FindStorage<A>();
        if (!a) return;
        for (auto it = a->begin(); it != a->end(); ++it)
        {
            fn(it->first, it->second);
        }
    }

    /// @brief 特定のコンポーネント型2種を持つエンティティを走査
    template<class A, class B, class Fn>
    void View(Fn&& fn)
    {
        auto* a = FindStorage<A>();
        auto* b = FindStorage<B>();
        if (!a || !b) return;

        // 少ない方を外側に
        if (a->size() <= b->size())
        {
            for (auto ita = a->begin(); ita != a->end(); ++ita)
            {
                auto itb = b->find(ita->first);
                if (itb != b->end())
                    fn(ita->first, ita->second, itb->second);
            }
        }
        else
        {
            for (auto itb = b->begin(); itb != b->end(); ++itb)
            {
                auto ita = a->find(itb->first);
                if (ita != a->end())
                    fn(itb->first, ita->second, itb->second);
            }
        }
    }

    // 読み取り専用ストレージ取得（Add/Removeしない系で使用）
    template<class T>
    const std::unordered_map<EntityId, T>* FindStorageConst() const
    {
        const ComponentType type = ComponentType(typeid(T));
        auto it = m_components.find(type);
        if (it == m_components.end()) return nullptr;
        return &static_cast<const ComponentStorage<T>*>(it->second.get())->data;
    }

    // ---- const View: 1種 ----
    template<class A, class Fn>
    void View(Fn&& fn) const
    {
        auto* a = FindStorageConst<A>();
        if (!a) return;
        for (auto it = a->begin(); it != a->end(); ++it)
        {
            fn(it->first, static_cast<const A&>(it->second));
        }
    }

    // ---- const View: 2種 ----
    template<class A, class B, class Fn>
    void View(Fn&& fn) const
    {
        auto* a = FindStorageConst<A>();
        auto* b = FindStorageConst<B>();
        if (!a || !b) return;

        if (a->size() <= b->size())
        {
            for (auto ita = a->begin(); ita != a->end(); ++ita)
            {
                auto itb = b->find(ita->first);
                if (itb != b->end())
                    fn(ita->first,
                        static_cast<const A&>(ita->second),
                        static_cast<const B&>(itb->second));
            }
        }
        else
        {
            for (auto itb = b->begin(); itb != b->end(); ++itb)
            {
                auto ita = a->find(itb->first);
                if (ita != a->end())
                    fn(itb->first,
                        static_cast<const A&>(ita->second),
                        static_cast<const B&>(itb->second));
            }
        }
    }

private:
    //-------------------------------------------------------------------------
    // ■ 内部構造
    //-------------------------------------------------------------------------

    /// @brief 具象コンポーネントストレージ
    template<class T>
    struct ComponentStorage final : public IComponentStorage
    {
        std::unordered_map<EntityId, T> data;
        void Remove(EntityId e) override { data.erase(e); }
        bool Has(EntityId e) const override { return data.count(e) != 0; }
    };

    /// @brief ストレージを取得（存在しなければ生成）
    template<class T>
    std::unordered_map<EntityId, T>& GetOrCreateStorage()
    {
        const ComponentType type = ComponentType(typeid(T));
        auto it = m_components.find(type);
        if (it == m_components.end())
        {
            std::unique_ptr<ComponentStorage<T>> ptr(new ComponentStorage<T>());
            ComponentStorage<T>* raw = ptr.get();
            m_components[type] = std::move(ptr);
            return raw->data;
        }
        return static_cast<ComponentStorage<T>*>(m_components[type].get())->data;
    }

    /// @brief ストレージを取得（存在しなければ nullptr）
    template<class T>
    std::unordered_map<EntityId, T>* FindStorage() const
    {
        const ComponentType type = ComponentType(typeid(T));
        auto it = m_components.find(type);
        if (it == m_components.end()) return nullptr;
        return &static_cast<ComponentStorage<T>*>(it->second.get())->data;
    }

private:
    //-------------------------------------------------------------------------
    // ■ メンバ
    //-------------------------------------------------------------------------
    std::unordered_map<ComponentType, std::unique_ptr<IComponentStorage>> m_components;
    EntityId m_nextEntity = kInvalidEntity;
};
