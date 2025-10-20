#pragma once
#include <DirectXMath.h>

/**
 * @file RigidbodyComponent.h
 * @brief 物理挙動の核となる「剛体」データ（簡易/線形のみ）
 * @details
 * - 2D/3D共通の最小要素。回転は必要になったら追加（角速度/慣性など）。
 * - 半固定：Semi-Implicit Euler（速度→位置の順）を想定。
 * - 重力は「物理の一部」としてここで制御（enabled & scale）。
 */
struct RigidbodyComponent
{
    DirectX::XMFLOAT3 velocity{ 0.f, 0.f, 0.f }; ///< 速度 (m/s)
    DirectX::XMFLOAT3 accumulatedForce{ 0.f, 0.f, 0.f }; ///< 今フレームの外力合算（入力/風/押し出し等）

    float inverseMass = 1.0f;      ///< 1/mass（0なら無限大の質量=静的）
    float linearDamping = 0.05f;   ///< 速度減衰（0-1程度）。空気抵抗的な減速

    bool  gravityEnabled = true;   ///< 重力適用の有無
    float gravityScale = 1.0f;   ///< 重力倍率（キャラごとの調整用）
};
