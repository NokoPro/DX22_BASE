#pragma once
#include "../IUpdateSystem.h"

/**
 * @file PhysicsSystem.h
 * @brief �O�͍��Z�{�d�́����x�����������[�V������(delta) ���v�Z����System
 * @details
 * - Semi-Implicit Euler ���̗p�F���x�X�V���ʒu�X�V�͌�i�iCollision�ŉ����j�Ɉς˂�B
 * - �{System�� Transform �𒼐ڂ����炸�AMotionDeltaComponent �� delta �������B
 * - �d�͂́u�����̈ꕔ�v�Ƃ��Ă����œK�p�iRigidbody �� gravityEnabled/scale ���Q�Ɓj�B
 */
class PhysicsSystem final : public IUpdateSystem 
{
public:
    /**
     * @param gravityY ���E�̏d�͉����xY (m/s^2, �������͕�)
     */
    explicit PhysicsSystem(float gravityY = -9.8f) : m_gravityY(gravityY) {}

    void Update(class World& world, float dt) override;

private:
    float m_gravityY;
};
