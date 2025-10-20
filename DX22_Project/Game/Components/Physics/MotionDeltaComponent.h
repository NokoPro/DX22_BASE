#pragma once
#include <DirectXMath.h>

/**
 * @file MotionDeltaComponent.h
 * @brief 今フレームで「移動したい量」を表す一時バッファ
 * @details
 * - PhysicsSystem が速度から算出して書き込み、CollisionSystem がこれを
 *   使ってスイープ/離散解決を行い、Transformを正す。
 * - 衝突後、CollisionSystem が消費してクリアする（または上書きする）。
 */
struct MotionDeltaComponent
{
    DirectX::XMFLOAT3 delta{ 0.f, 0.f, 0.f }; ///< フレーム内の希望移動量（m）
};
