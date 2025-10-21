#pragma once
#include "../IRenderSystem.h"

/**
 * @file ObbDebugRenderSystem.h
 * @brief ObbCollider を半透明の薄い緑 + 緑ワイヤで描画するデバッグSystem
 * @details
 * - Transform + ObbCollider を持つ全エンティティを描く。
 * - Render(const World&) 準拠。表示ON/OFFは外部フラグポインタで制御。
 */
class ObbDebugRenderSystem final : public IRenderSystem
{
public:
    explicit ObbDebugRenderSystem(const bool* enabled = nullptr)
        : m_enabled(enabled) {
    }

    /// @brief OBB を描画（半透明の塗り + 輪郭ライン）
    void Render(const class World& world) override;

private:
    const bool* m_enabled; ///< 表示ON/OFFフラグ（nullptrなら常に描画）
};
