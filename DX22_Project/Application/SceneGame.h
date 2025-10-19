#pragma once
#include "../System/Scene.h"
#include "../System/Input.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Geometory.h"
#include "../System/Model.h"

#include "../Game/ECS.h"
#include "../Game/World.h"
#include "../Game/Components/TransformComponent.h"
#include "../Game/Components/ModelRendererComponent.h"
#include "../Game/Systems/SystemRegistry.h"
#include "../Game/Systems/Update/OrbitCameraSystem.h"
#include "../Game/Systems/Update/SpinSystem.h"
#include "../Game/Systems/Render/ModelRenderSystem.h"
#include "../Game/Systems/Render/DebugGridRenderSystem.h"
#include <DirectXMath.h>

/**
 * @file SceneGame.h
 * @brief ゲーム本編シーン（ECS：Model を Transform+ModelRenderer で描画）
 */
class SceneGame : public Scene
{
public:
    SceneGame();
    ~SceneGame();

    /** @brief フレーム更新（入力・カメラ・ロジック） */
    void Update() final;

    /** @brief フレーム描画（モデル・デバッグ） */
    void Draw() final;

private:
    // === ECS中枢 ===
    World           m_world;
    SystemRegistry  m_sys;

    // === カメラ（Update System） ===
    OrbitCameraSystem* m_cam = nullptr;

    // === 描画（Render System参照） ===
    ModelRenderSystem* m_drawModel = nullptr;
    bool                m_showGrid = true;

    // === 資産 ===
    Model m_mushroom;

    // === 入力用トグル ===
    bool m_pauseSpin = false;
};
