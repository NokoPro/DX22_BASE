#pragma once
#include "../IUpdateSystem.h"

/**
 * @file PhysicsSystem.h
 * @brief 外力合算＋重力→速度→減衰→モーション量(delta) を計算するSystem
 * @details
 * - Semi-Implicit Euler を採用：速度更新→位置更新は後段（Collisionで解決）に委ねる。
 * - 本Systemは Transform を直接いじらず、MotionDeltaComponent に delta を書く。
 * - 重力は「物理の一部」としてここで適用（Rigidbody の gravityEnabled/scale を参照）。
 */
class PhysicsSystem final : public IUpdateSystem 
{
public:
    /**
     * @param gravityY 世界の重力加速度Y (m/s^2, 下向きは負)
     */
    explicit PhysicsSystem(float gravityY = -9.8f) : m_gravityY(gravityY) {}

    void Update(class World& world, float dt) override;

private:
    float m_gravityY;
};
