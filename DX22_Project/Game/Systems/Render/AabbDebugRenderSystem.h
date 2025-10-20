#pragma once
#include "../IRenderSystem.h"

/**
 * @file AabbDebugRenderSystem.h
 * @brief AabbCollider を「実箱＋接触時は薄緑ワイヤ」描画するデバッグSystem
 * @details
 * - Transform + AabbCollider を持つ全エンティティを描く。
 * - Render(const World&) 準拠。表示ON/OFFは外部フラグポインタで制御。
 */
class AabbDebugRenderSystem final : public IRenderSystem {
public:
    /**
     * @param enabled 表示ON/OFFを外部制御するフラグ（nullptr可）
     */
    explicit AabbDebugRenderSystem(const bool* enabled = nullptr)
        : m_enabled(enabled) {
    }

    /// @brief AABBを描画（実箱）。接触中は薄緑ワイヤを重ねる。
    void Render(const class World& world) override;

private:
    const bool* m_enabled; ///< 表示ON/OFFフラグ（nullptrなら常に描画）
};
