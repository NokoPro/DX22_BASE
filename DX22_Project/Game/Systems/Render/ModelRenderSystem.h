#pragma once
#include "../IRenderSystem.h"
#include <DirectXMath.h>

/**
 * @file ModelRenderSystem.h
 * @brief Transform+ModelRenderer を描画するレンダリングシステム
 * @details
 * - View/Proj は外部（カメラSystem）から受け取る。
 * - ライティング等の共有設定は静的関数で提供。
 */
class ModelRenderSystem final : public IRenderSystem
{
public:
    /** @brief ビュー・プロジェクションを設定（転置済みの想定） */
    void SetViewProj(const DirectX::XMFLOAT4X4& V, const DirectX::XMFLOAT4X4& P) { m_V = V; m_P = P; }

    /** @brief 描画実行 */
    void Render(const class World& world) override;  // ← Draw→Render / const World& に合わせる

    /** @brief 起動時に一度だけのライティング初期設定 */
    static void ApplyDefaultLighting(float camY, float camRadius);

private:
    DirectX::XMFLOAT4X4 m_V{}, m_P{};
};
