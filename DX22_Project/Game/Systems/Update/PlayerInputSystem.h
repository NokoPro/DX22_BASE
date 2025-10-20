#pragma once
#include "../IUpdateSystem.h"

/**
 * @file PlayerInputSystem.h
 * @brief PC�L�[�{�[�h + XInput�Q�[���p�b�h������͂�ǂݎ��ACharacterController �ɔ��f����V�X�e��
 * @details
 * - �������͂� XZ ���ʁiX=���E, Z=�O��j�B�L�[�{�[�hWASD�ƍ��X�e�B�b�N�𓝍��B
 * - �W�����v�� Space �܂��� �p�b�h��A�{�^���i�ڒn����͕ʃV�X�e���ōs���z��j�B
 * - �u���͂̈Ӑ}�v�������X�V���A���W�⑬�x�͕ύX���Ȃ��i�Ӗ������j�B
 */
class PlayerInputSystem final : public IUpdateSystem {
public:
    /**
     * @param stickDeadZone ���X�e�B�b�N�̃f�b�h�]�[���i0.0 - 1.0�j
     */
    explicit PlayerInputSystem(float stickDeadZone = 0.2f)
        : m_deadZone(stickDeadZone) {
    }

    /// @brief 1�t���[�����̓��͎擾
    void Update(class World& world, float dt) override;

private:
    float m_deadZone; ///< ���X�e�B�b�N�̃f�b�h�]�[���i�K�i����j
};
