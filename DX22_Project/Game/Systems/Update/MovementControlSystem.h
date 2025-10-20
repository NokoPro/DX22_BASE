#pragma once
#include "../IUpdateSystem.h"

/**
 * @file MovementControlSystem.h
 * @brief ���́i�Ӑ}�j����ڕW�������x�����A���̂ցu�́v��^����System
 * @details
 * - ���������iXZ�j�̂ݐ���BY�i�����j�̓W�����v���̗��U�A�N�V�����ň����B
 * - ���x����́u�ڕW���x�ɋ߂Â����߂̉����x�v���v�Z���AForce=mass*accel �Ƃ��ĉ�����B
 * - �W�����v�͐ڒn���̂ݗL���ȃC���p���X�i���x�ɒ��ډ��Z�j�ŗ^����B
 */
class MovementControlSystem final : public IUpdateSystem {
public:
    /**
     * @param walkSpeed   �ʏ���s�̍ő吅�����x (m/s)
     * @param runSpeed    ����C�����̍ő吅�����x (m/s)
     * @param maxAccelXZ  ���������̍ő�����x (m/s^2)
     * @param jumpSpeed   �W�����v���� (m/s, �������)
     */
    MovementControlSystem(float walkSpeed = 3.0f,
        float runSpeed = 5.0f,
        float maxAccelXZ = 25.0f,
        float jumpSpeed = 5.0f)
        : m_walkSpeed(walkSpeed), m_runSpeed(runSpeed),
        m_maxAccelXZ(maxAccelXZ), m_jumpSpeed(jumpSpeed) {
    }

    void Update(class World& world, float dt) override;

private:
    float m_walkSpeed;
    float m_runSpeed;
    float m_maxAccelXZ;
    float m_jumpSpeed;
};
