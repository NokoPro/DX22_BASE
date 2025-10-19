#pragma once
#include <DirectXMath.h>

/**
 * @brief 位置・回転(度)・スケールを表す基本トランスフォーム
 * @details
 * - 依存は DirectXMath のみ（軽量）
 * - 回転は度数法で保持し、使用時にラジアンへ変換
 */
struct TransformComponent
{
    DirectX::XMFLOAT3 position{ 0,0,0 };
    DirectX::XMFLOAT3 rotationDeg{ 0,0,0 };
    DirectX::XMFLOAT3 scale{ 1,1,1 };
};

// 既存コード互換のための別名（今までの Transform をそのまま使える）
using Transform = TransformComponent;
