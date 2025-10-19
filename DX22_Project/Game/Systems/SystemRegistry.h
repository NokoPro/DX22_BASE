#pragma once
#include <memory>
#include <vector>
#include "IUpdateSystem.h"
#include "IRenderSystem.h"

/**
 * @file SystemRegistry.h
 * @brief Update�n��Render�n�𕪗����s���郌�W�X�g��
 * @details
 * - �o�^�� = ���s���i�ˑ��֌W������ꍇ�͓o�^���Ő���j�B
 * - AddUpdate<T>(), AddRender<T>() �ō\�z�B Tick()/Render() �Ŏ��s�B
 */
class SystemRegistry
{
public:
    /** @brief Update�V�X�e����ǉ��i�o�^�������s���j */
    template<class T, class... Args>
    T& AddUpdate(Args&&... args) {
        static_assert(std::is_base_of<IUpdateSystem, T>::value, "T must be IUpdateSystem");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        m_updates.emplace_back(std::move(ptr));
        return ref;
    }

    /** @brief Render�V�X�e����ǉ��i�o�^�������s���j */
    template<class T, class... Args>
    T& AddRender(Args&&... args) {
        static_assert(std::is_base_of<IRenderSystem, T>::value, "T must be IRenderSystem");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        m_renders.emplace_back(std::move(ptr));
        return ref;
    }

    /** @brief Update�p�X�����s */
    void Tick(class World& world, float dt);

    /** @brief Render�p�X�����s */
    void Render(class World& world);

private:
    std::vector<std::unique_ptr<IUpdateSystem>> m_updates; ///< �X�V�p�X�Q
    std::vector<std::unique_ptr<IRenderSystem>> m_renders; ///< �`��p�X�Q
};
