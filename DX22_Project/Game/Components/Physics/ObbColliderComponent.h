#pragma once
#include <DirectXMath.h>

/**
 * @file ObbColliderComponent.h
 * @brief OBB（Oriented Bounding Box）コライダ
 * @details
 * - Transform のワールド回転に加え、OBB自身のローカル回転も持てます。
 * - offset は Transform 原点からのローカル平行移動。
 * - halfExtents はローカル軸に沿ったハーフサイズ。
 * - isStatic=true は静的ジオメトリ（床/壁など）として扱う想定。
 */
struct ObbColliderComponent
{
    DirectX::XMFLOAT3 offset{ 0.f, 0.f, 0.f };      ///< ローカル中心のオフセット
    DirectX::XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f }; ///< ローカル軸のハーフサイズ (x,y,z)
    DirectX::XMFLOAT3 rotationDeg{ 0.f, 0.f, 0.f }; ///< OBBローカル回転（Transform の回転にさらに足す）
    bool isStatic = false;                         ///< 静的（無限質量）かどうか
};
