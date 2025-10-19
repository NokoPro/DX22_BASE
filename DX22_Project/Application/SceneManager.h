#pragma once
#include <memory>
#include "../System/Scene.h"

/**
 * @file SceneManager.h
 * @brief �P��A�N�e�B�u�V�[�����Ǘ����A�ؑցE�X�V�E�`��𒇉��Ǘ��N���X
 * @details
 * - �{�Ǘ��́u���1�����v�A�N�e�B�u�� Scene ��ێ�����B
 * - Change<T>(args...) �ŃV�[���ؑցi�Â��V�[���͔j���j�B
 * - Update()/Draw() �� Scene �� RootUpdate()/RootDraw() �o�R�ŌĂԁB
 */
class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    /** @brief �V�[���ؑցi�Â��V�[���͔j���j */
    template<class T, class... Args>
    void Change(Args&&... args) {
        static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");
        m_current.reset(new T(std::forward<Args>(args)...));
    }

    /** @brief ���݂̃V�[�����X�V */
    void Update() {
        if (m_current) m_current->RootUpdate();
    }

    /** @brief ���݂̃V�[����`�� */
    void Draw() {
        if (m_current) m_current->RootDraw();
    }

    /** @brief ���݂̃V�[���擾�i�K�v�Ȃ�L���X�g���Ďg���j */
    Scene* Current() const { return m_current.get(); }

private:
    std::unique_ptr<Scene> m_current; ///< �A�N�e�B�u�V�[��
};
