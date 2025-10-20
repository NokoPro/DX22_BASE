#pragma once
#include "../IUpdateSystem.h"

/**
 * @file MovementControlSystem.h
 * @brief 入力（意図）から目標水平速度を作り、剛体へ「力」を与えるSystem
 * @details
 * - 水平方向（XZ）のみ制御。Y（垂直）はジャンプ等の離散アクションで扱う。
 * - 速度制御は「目標速度に近づくための加速度」を計算し、Force=mass*accel として加える。
 * - ジャンプは接地中のみ有効なインパルス（速度に直接加算）で与える。
 */
class MovementControlSystem final : public IUpdateSystem {
public:
    /**
     * @param walkSpeed   通常歩行の最大水平速度 (m/s)
     * @param runSpeed    走り修飾時の最大水平速度 (m/s)
     * @param maxAccelXZ  水平方向の最大加速度 (m/s^2)
     * @param jumpSpeed   ジャンプ初速 (m/s, 上向き正)
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
