#pragma once
#include "../IUpdateSystem.h"
#include <DirectXMath.h>

/**
 * @file FollowCameraSystem.h
 * @brief ターゲット追従 + 軸ロック + スムーズ追従を行うカメラ更新システム
 * @details
 * - ActiveCameraTag + CameraRigComponent + Transform を持つエンティティを対象に動作。
 * - 追従先 target の Transform を参照し、カメラの Transform を更新。
 * - View/Projection 行列を生成して、外部へ提供（ModelRenderSystem などへ配線）。
 * - Geometory の View/Proj もここで更新しておくとデバッグ描画が同期する。
 */
class FollowCameraSystem final : public IUpdateSystem {
public:
    FollowCameraSystem() = default;

    /// @brief 1フレーム分の更新（追従/スムージング/行列更新）
    void Update(class World& world, float dt) override;

    /// @brief 転置済みの View 行列を取得（描画用）
    const DirectX::XMFLOAT4X4& GetView() const { return m_V; }

    /// @brief 転置済みの Projection 行列を取得（描画用）
    const DirectX::XMFLOAT4X4& GetProj() const { return m_P; }

private:
    DirectX::XMFLOAT4X4 m_V{}; ///< 転置済み View
    DirectX::XMFLOAT4X4 m_P{}; ///< 転置済み Projection
};
