#pragma once
#include "../IRenderSystem.h"

/**
 * @file DebugGridRenderSystem.h
 * @brief XZ平面のグリッドと軸を描画するだけのデバッグ用レンダリングシステム
 * @details
 * - F1トグル等の外部フラグポインタを受け取り、それに従って描画。
 */
class DebugGridRenderSystem final : public IRenderSystem
{
public:
    explicit DebugGridRenderSystem(bool* flagPtr) : m_flag(flagPtr) {}
    void Render(const class World& world) override;  // ← Draw→Render / const World& に合わせる

private:
    bool* m_flag; ///< 表示フラグ（外部管理）
};
