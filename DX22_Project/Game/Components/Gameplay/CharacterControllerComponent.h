#pragma once
#include <DirectXMath.h>

/**
 * @file CharacterControllerComponent.h
 * @brief プレイヤー/AIの「操作意図」を表す軽量コンポーネント
 * @details
 * - 入力はXZ平面を想定（X=左右, Z=前後）。Yはジャンプなど離散アクションで扱う。
 * - ここでは「何をしたいか」だけを保持し、座標や速度の直接変更はしない。
 * - 実際の物理量（速度/位置）への影響は MovementControlSystem / PhysicsSystem が行う。
 */
struct CharacterControllerComponent
{
    DirectX::XMFLOAT2 moveInput{ 0.f, 0.f }; ///< -1 - +1程度の入力。未正規化で可
    bool jumpPressed = false;               ///< このフレームでジャンプが押されたか
    bool runModifier = false;               ///< Shift等の走り修飾（任意）
};
