#pragma once
#include "../IUpdateSystem.h"

/**
 * @file SpinSystem.h
 * @brief �w�莲�ŉ�]�����邾���̃f���p�X�V�V�X�e��
 * @details
 * - TransformComponent �� rotationDeg �𖈃t���[���X�V�B
 */
class SpinSystem final : public IUpdateSystem 
{
public:
    explicit SpinSystem(float degPerSecY = 45.f) : ySpeed(degPerSecY) {}

    void Update(class World& world, float dt) override;

    float ySpeed; ///< Y���̖��b��]�p�i�x�j
};
