#pragma once
#include "ECS.h"
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <cassert>
#include <memory>      // �� �ǉ��Funique_ptr
#include <typeindex>   // �O�̂��߁iComponentType�̎�������ŕs�v�Ȃ�O����OK�j

/**
 * @file World.h
 * @brief Entity �� Component �̐����E�Ǘ����s�����S�N���X�iC++14�Ή��j
 * @details
 * - Add<T>(entity, args...) / Get<T>(entity) / Has<T>(entity)
 * - View<A,B>(lambda) ��2��R���|�[�l���g�����G���e�B�e�B�փA�N�Z�X
 * - C++14�Ή��̂��ߍ\���������͎g�p���Ă��܂���
 */
class World
{
public:
    World() = default;
    ~World() = default;

    //-------------------------------------------------------------------------
    // �� Entity
    //-------------------------------------------------------------------------

    /// @brief �V�����G���e�B�e�B�𐶐�
    EntityId Create()
    {
        return ++m_nextEntity;
    }

    /// @brief �G���e�B�e�B���폜�i�֘A�R���|�[�l���g���폜�j
    void Destroy(EntityId e)
    {
        // C++14 �Ȃ̂ō\���������͎g��Ȃ�
        for (auto it = m_components.begin(); it != m_components.end(); ++it)
        {
            it->second->Remove(e);
        }
    }

    //-------------------------------------------------------------------------
    // �� Component ����
    //-------------------------------------------------------------------------

    /// @brief �R���|�[�l���g��ǉ��i���݂���Ώ㏑���j
    template<class T, class... Args>
    T& Add(EntityId e, Args&&... args)
    {
        auto& store = GetOrCreateStorage<T>();
        return store[e] = T{ std::forward<Args>(args)... };
    }

    /// @brief �R���|�[�l���g���擾�i���݂��Ȃ���� nullptr�j
    template<class T>
    T* Get(EntityId e)
    {
        auto* s = FindStorage<T>();
        if (!s) return nullptr;
        auto it = s->find(e);
        return (it == s->end()) ? nullptr : &it->second;
    }

    /// @brief �R���|�[�l���g���폜
    template<class T>
    void Remove(EntityId e)
    {
        auto* s = FindStorage<T>();
        if (s) s->erase(e);
    }

    /// @brief �R���|�[�l���g�̗L�����m�F
    template<class T>
    bool Has(EntityId e) const
    {
        auto* s = FindStorage<T>();
        if (!s) return false;
        return s->count(e) != 0;
    }

    //-------------------------------------------------------------------------
    // �� View
    //-------------------------------------------------------------------------

    /// @brief ����̃R���|�[�l���g�^1������G���e�B�e�B�𑖍�
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

    /// @brief ����̃R���|�[�l���g�^2������G���e�B�e�B�𑖍�
    template<class A, class B, class Fn>
    void View(Fn&& fn)
    {
        auto* a = FindStorage<A>();
        auto* b = FindStorage<B>();
        if (!a || !b) return;

        // ���Ȃ������O����
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

    // �ǂݎ���p�X�g���[�W�擾�iAdd/Remove���Ȃ��n�Ŏg�p�j
    template<class T>
    const std::unordered_map<EntityId, T>* FindStorageConst() const
    {
        const ComponentType type = ComponentType(typeid(T));
        auto it = m_components.find(type);
        if (it == m_components.end()) return nullptr;
        return &static_cast<const ComponentStorage<T>*>(it->second.get())->data;
    }

    // ---- const View: 1�� ----
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

    // ---- const View: 2�� ----
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
    // �� �����\��
    //-------------------------------------------------------------------------

    /// @brief ��ۃR���|�[�l���g�X�g���[�W
    template<class T>
    struct ComponentStorage final : public IComponentStorage
    {
        std::unordered_map<EntityId, T> data;
        void Remove(EntityId e) override { data.erase(e); }
        bool Has(EntityId e) const override { return data.count(e) != 0; }
    };

    /// @brief �X�g���[�W���擾�i���݂��Ȃ���ΐ����j
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

    /// @brief �X�g���[�W���擾�i���݂��Ȃ���� nullptr�j
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
    // �� �����o
    //-------------------------------------------------------------------------
    std::unordered_map<ComponentType, std::unique_ptr<IComponentStorage>> m_components;
    EntityId m_nextEntity = kInvalidEntity;
};
