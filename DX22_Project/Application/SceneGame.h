#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

#include "../System/Scene.h"
#include "../System/Model.h"
#include "../System/Geometory.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Input.h"
#include <DirectXMath.h>

/**
 * @brief ゲーム本編シーン（まずはモデル1体をテクスチャ付きで描画）
 * @details
 * - 起動時に FBX/OBJ を読み込み（Assimp）
 * - ShaderList の既定シェーダ（Lambert）を割り当て
 * - 毎フレーム、カメラを回転させてモデルを描画
 * - デバッグ用にグリッドを表示（F1でトグル）
 */
class SceneGame : public Scene
{
public:
    SceneGame();
    ~SceneGame();

    /** @brief 更新処理（カメラ操作など） */
    void Update() final;

    /** @brief 描画処理（モデル＋デバッググリッド） */
    void Draw() final;

private:
    /// モデル本体
    Model m_model;

    /// 行列（world, view, proj）
    DirectX::XMFLOAT4X4 m_wvp[3];

    /// カメラ角度（水平オービット用）
    float m_camAngle = 0.0f;

    /// デバッググリッド表示
    bool m_showGrid = true;

    /// 読み込み成功フラグ
    bool m_loaded = false;

private:
    /** @brief View/Projection の更新 */
    void UpdateCamera(float dt);

    /** @brief World/View/Proj を ShaderList に反映 */
    void ApplyWVP();

    /** @brief デフォルトライティングを設定 */
    void ApplyLighting();
};

#endif // __SCENE_GAME_H__
