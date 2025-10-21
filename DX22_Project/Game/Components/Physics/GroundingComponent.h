#pragma once
#include <DirectXMath.h>

/**
 * @file GroundingComponent.h
 * @brief 接地状態のキャッシュ（衝突解決が更新する）
 * @details
 * - CollisionSystem が「接触法線を上向きとみなせるか」を判定して更新。
 * - 物理/移動側はこのフラグでジャンプ許可や垂直速度のクランプを行う。
 */
struct GroundingComponent
{
    bool grounded = false;                          ///< 接地しているか
    DirectX::XMFLOAT3 groundNormal{ 0.f, 1.f, 0.f };  ///< 直近の接地法線
    float timeSinceGrounded = 1e9f;                 ///< 最後に接地してからの秒数

    bool hasContact = false;
    DirectX::XMFLOAT3 contactNormal{ 0,0,0 };
};
