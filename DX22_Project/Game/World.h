/**
 * @file World.h
 * @brief ECS�ɂ�����World�N���X��`
 * @details
 * Entity(�G���e�B�e�B)�̐����E�j���AComponent(�R���|�[�l���g)�̊Ǘ��A
 * �����View(�V�X�e�������p�̑���)��S���N���X�ł��B
 *
 * �ˑ��֌W�F
 *   - ECS.h�iEntityId, IComponentStorage, ComponentType�̒�`���܂ށj
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
  * @brief �e�^T��p�̃R���|�[�l���g�X�g���[�W
  * @tparam T �Ǘ��Ώۂ̃R���|�[�l���g�^
  */
template<class T>
struct ComponentStorage final : IComponentStorage {
    /// �G���e�B�e�BID���L�[�Ƃ���R���|�[�l���g�̘A�z�z��
    std::unordered_map<EntityId, T> data;

    /// �w��G���e�B�e�B�̃R���|�[�l���g���폜
    void Remove(EntityId e) override { data.erase(e); }

    /// �w��G���e�B�e�B���R���|�[�l���g�������m�F
    bool Has(EntityId e) const override { return data.find(e) != data.end(); }
};

/**
 * @class World
 * @brief ECS�S�̂��Ǘ����钆�S�N���X
 * @details
 * �e�R���|�[�l���g�^���Ƃ̃X�g���[�W��ێ����A
 * �G���e�B�e�B�̐����E�폜�A�R���|�[�l���g�̒ǉ�/�擾/�폜�A
 * �����ĕ����R���|�[�l���g��g�ݍ��킹�đ���(View)���s���@�\��񋟂��܂��B
 */
class World {
public:
    /// @brief �f�t�H���g�R���X�g���N�^
    World() = default;

    //--------------------------------------------------------------------------
    // �G���e�B�e�B�Ǘ�
    //--------------------------------------------------------------------------

    /**
     * @brief �V�����G���e�B�e�B�𐶐�
     * @return �������ꂽ�G���e�B�e�BID
     */
    EntityId Create() {
        if (m_next == kInvalidEntity) m_next = 1;
        return m_next++;
    }

    /**
     * @brief �w��G���e�B�e�B��j��
     * @param e �폜����G���e�B�e�BID
     * @details �o�^����Ă���S�ẴX�g���[�W����Ώۂ̃G���e�B�e�B���폜���܂��B
     */
    void Destroy(EntityId e) {
        for (auto& kv : m_storages) {
            kv.second->Remove(e);
        }
    }

    //--------------------------------------------------------------------------
    // �R���|�[�l���g����
    //--------------------------------------------------------------------------

    /**
     * @brief �R���|�[�l���g��ǉ��܂��͏㏑��
     * @tparam T �ǉ�����R���|�[�l���g�^
     * @param e �ΏۃG���e�B�e�BID
     * @param args �R���X�g���N�^����
     * @return �ǉ��܂��͏㏑�����ꂽ�R���|�[�l���g�Q��
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
     * @brief �w��G���e�B�e�B���R���|�[�l���g��ێ����Ă��邩�m�F
     * @tparam T �R���|�[�l���g�^
     * @param e �G���e�B�e�BID
     * @return true:�ێ����Ă��� / false:�ێ����Ă��Ȃ�
     */
    template<class T>
    bool Has(EntityId e) const {
        const auto* s = find<T>();
        return s && s->count(e) != 0;
    }

    /**
     * @brief �R���|�[�l���g���擾�i��const�Łj
     * @tparam T �R���|�[�l���g�^
     * @param e �G���e�B�e�BID
     * @return �Q��
     * @note ���݂��Ȃ��ꍇ��assert�Œ�~���܂��B
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
     * @brief �R���|�[�l���g���擾�iconst�Łj
     * @tparam T �R���|�[�l���g�^
     * @param e �G���e�B�e�BID
     * @return const�Q��
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
     * @brief �R���|�[�l���g���폜
     * @tparam T �R���|�[�l���g�^
     * @param e �ΏۃG���e�B�e�BID
     */
    template<class T>
    void Remove(EntityId e) {
        auto* s = find<T>();
        if (s) s->erase(e);
    }

    //--------------------------------------------------------------------------
    // View�����i�����_�ŕ����R���|�[�l���g�𑖍��j
    //--------------------------------------------------------------------------

    /**
     * @brief ��const��View����
     * @tparam A, Rest... �ΏۃR���|�[�l���g�^�Q
     * @tparam Fn �Ăяo���֐��^
     * @param fn �Ăяo�������_�i��: [](EntityId, A&, Rest&...){}�j
     * @details
     * �w�肵���S�ẴR���|�[�l���g�����G���e�B�e�B�𑖍����A�֐����Ăяo���܂��B
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
     * @brief const��View����
     * @tparam A, Rest... �ΏۃR���|�[�l���g�^�Q
     * @tparam Fn �Ăяo���֐��^
     * @param fn �Ăяo�������_�i��: [](EntityId, const A&, const Rest&...){}�j
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
    // �����w���p�֐��Q
    //--------------------------------------------------------------------------

    /**
     * @brief �w��^�̃X�g���[�W���擾�i���݂��Ȃ���ΐ����j
     * @tparam T �R���|�[�l���g�^
     * @return �X�g���[�W�ւ̎Q��
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
     * @brief �w��^�X�g���[�W�������i��const�Łj
     * @tparam T �R���|�[�l���g�^
     * @return ������΃|�C���^�A�Ȃ����nullptr
     */
    template<class T>
    std::unordered_map<EntityId, T>* find() {
        ComponentType key = std::type_index(typeid(T));
        auto it = m_storages.find(key);
        if (it == m_storages.end()) return nullptr;
        return &static_cast<ComponentStorage<T>*>(it->second.get())->data;
    }

    /**
     * @brief �w��^�X�g���[�W�������iconst�Łj
     * @tparam T �R���|�[�l���g�^
     * @return ������΃|�C���^�A�Ȃ����nullptr
     */
    template<class T>
    const std::unordered_map<EntityId, T>* find() const {
        ComponentType key = std::type_index(typeid(T));
        auto it = m_storages.find(key);
        if (it == m_storages.end()) return nullptr;
        return &static_cast<const ComponentStorage<T>*>(it->second.get())->data;
    }

    /**
     * @brief �ϒ��p�b�N�� "��" �P�[�X���󂯂閖�[��
     * @return ���true
     */
    template<class... Ts>
    typename std::enable_if<sizeof...(Ts) == 0, bool>::type
        has_all_rest(EntityId) const {
        return true;
    }

    /**
     * @brief �R���|�[�l���g��S�ĕێ����Ă��邩���ċA�I�Ɋm�F
     * @tparam T1, Ts... �ΏۃR���|�[�l���g�^�Q
     * @param e �G���e�B�e�BID
     * @return �S�ĕێ����Ă����true
     */
    template<class T1, class... Ts>
    bool has_all_rest(EntityId e) const {
        return Has<T1>(e) && has_all_rest<Ts...>(e);
    }

private:
    /// �e�R���|�[�l���g�^�ɑΉ�����X�g���[�W�}�b�v
    std::unordered_map<ComponentType, std::unique_ptr<IComponentStorage>> m_storages;

    /// ���Ɋ��蓖�Ă�G���e�B�e�BID
    EntityId m_next = kInvalidEntity;
};
