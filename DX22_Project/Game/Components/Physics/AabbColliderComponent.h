#pragma once
#include <DirectXMath.h>

/**
 * @file AabbColliderComponent.h
 * @brief AABB（軸平行バウンディングボックス）コライダ
 * @details
 * - Transform（ワールド座標）の上に「オフセット+ハーフサイズ」で配置する設計。
 * - isStatic=true のものは質量無限相当として動かさない（地面/壁など）。
 * - マテリアルや反発/摩擦は必要に応じ別コンポーネントで拡張。
 */
struct AabbColliderComponent
{
    DirectX::XMFLOAT3 offset{ 0.f, 0.f, 0.f };      ///< Transform原点からのオフセット
    DirectX::XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f }; ///< ハーフサイズ（x,y,z）
    bool isStatic = false;                         ///< 静的（地面/壁）かどうか
};
