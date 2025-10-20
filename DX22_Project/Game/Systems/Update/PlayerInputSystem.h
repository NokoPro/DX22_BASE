#pragma once
#include "../IUpdateSystem.h"

/**
 * @file PlayerInputSystem.h
 * @brief PCキーボード + XInputゲームパッドから入力を読み取り、CharacterController に反映するシステム
 * @details
 * - 方向入力は XZ 平面（X=左右, Z=前後）。キーボードWASDと左スティックを統合。
 * - ジャンプは Space または パッドのAボタン（接地判定は別システムで行う想定）。
 * - 「入力の意図」だけを更新し、座標や速度は変更しない（責務分離）。
 */
class PlayerInputSystem final : public IUpdateSystem {
public:
    /**
     * @param stickDeadZone 左スティックのデッドゾーン（0.0 - 1.0）
     */
    explicit PlayerInputSystem(float stickDeadZone = 0.2f)
        : m_deadZone(stickDeadZone) {
    }

    /// @brief 1フレーム分の入力取得
    void Update(class World& world, float dt) override;

private:
    float m_deadZone; ///< 左スティックのデッドゾーン（規格化後）
};
