#pragma once
#include "../IUpdateSystem.h"
#include <DirectXMath.h>

/**
 * @file OrbitCameraSystem.h
 * @brief カメラを周回させる更新システム（View/Projを保持）
 * @details
 * - マウス操作等を入れたい場合もここに実装。
 * - レンダリング側へは GetView()/GetProj() で行列提供。
 */
class OrbitCameraSystem final : public IUpdateSystem 
{
public:
    /**
     * @brief コンストラクタ
     * @param radius 周回半径
     * @param height カメラ高さ
     * @param fovYDeg 垂直FOV（度）
     * @param aspect 画面アスペクト
     * @param zNear 近クリップ
     * @param zFar 遠クリップ
     */
    OrbitCameraSystem(float radius = 5.f, float height = 3.f,
        float fovYDeg = 60.f, float aspect = 16.f / 9.f,
        float zNear = 0.1f, float zFar = 1000.f);

    void Update(class World& world, float dt) override;

    /** @brief View行列（転置済み）取得 */
    const DirectX::XMFLOAT4X4& GetView() const { return m_V; }

    /** @brief Proj行列（転置済み）取得 */
    const DirectX::XMFLOAT4X4& GetProj() const { return m_P; }

    /** @brief カメラ高さの取得（ライティング設定などで使用） */
    float CamY() const { return m_height; }

    /** @brief カメラ半径の取得（ライティング設定などで使用） */
    float CamRadius() const { return m_radius; }

private:
    float m_angle = 0.f;
    float m_radius, m_height, m_fovYDeg, m_aspect, m_zNear, m_zFar;
    DirectX::XMFLOAT4X4 m_V{}, m_P{};
};
