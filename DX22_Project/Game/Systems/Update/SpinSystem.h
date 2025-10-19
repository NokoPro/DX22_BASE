#pragma once
#include "../IUpdateSystem.h"

/**
 * @file SpinSystem.h
 * @brief 指定軸で回転させるだけのデモ用更新システム
 * @details
 * - TransformComponent の rotationDeg を毎フレーム更新。
 */
class SpinSystem final : public IUpdateSystem 
{
public:
    explicit SpinSystem(float degPerSecY = 45.f) : ySpeed(degPerSecY) {}

    void Update(class World& world, float dt) override;

    float ySpeed; ///< Y軸の毎秒回転角（度）
};
