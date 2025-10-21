/**
 * @file SceneGame.cpp
 * @brief ゲームプレイシーンの実装
 * @details
 * このファイルでは、シーン固有の資産ロード、ECSシステム構築、エンティティスポーン、
 * およびフレームごとの更新・描画ロジックを実装します。
 */
#include "SceneGame.h"
#include <cmath>
#include "Game/Prefabs/PlayerPrefab.h"
#include "Game/Prefabs/StaticBoxPrefab.h"

 // ECSコンポーネント (m_world.Add<> のために必要)
#include "../Game/Components/TransformComponent.h"
#include "../Game/Components/ModelRendererComponent.h"
#include "../Game/Components/Camera/ActiveCameraTag.h"
#include "../Game/Components/Camera/CameraRigComponent.h"

using namespace DirectX;

/**
 * @brief SceneGame クラスのコンストラクタ
 * @details
 * 資産ロード、Prefab登録、初期エンティティのスポーン、ECSシステムの構築を行います。
 */
SceneGame::SceneGame()
{
    // --------------- 資産ロード（モデル＋シェーダ） ---------------
    const bool ok = m_mushroom.Load("Assets/Model/slime.fbx", 1.3f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    const bool good = m_ground.Load("Assets/Model/KayKit_Platformer_Pack_1.0_FREE/Assets/fbx/blue/platform_1x1x1_blue.fbx", 2.0f, Model::Flip::None);
    m_ground.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_ground.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // --------------- Prefab 登録 ---
    // Player（モデルをキャプチャ）
    {
        Prefabs::PlayerConfig pcfg;
        // プレイヤーのモデル情報とコリジョン情報を設定
        pcfg.model = ok ? &m_mushroom : nullptr;
        pcfg.colliderHalf = { 0.4f, 0.35f, 0.4f };
        pcfg.colliderOffset = { 0.f, 0.35f, 0.f };
        pcfg.inverseMass = 1.0f;
        pcfg.linearDamping = 0.05f;
        pcfg.gravityEnabled = true;
        pcfg.gravityScale = 1.0f;

        // 修正: Prefab関数に引数 pcfg を渡す
        m_prefabs.Register("Player", Prefabs::MakePlayerPrefab(pcfg));
    }

    // StaticBox（床/壁）
    m_prefabs.Register("StaticBox", Prefabs::MakeStaticBoxPrefab(good ? &m_ground : nullptr));

    // --- スポーン ---
    // 床：中心(0,-0.25,0)、ハーフ(5,0.25,5)
    m_prefabs.Spawn("StaticBox", m_world,
        {
            DirectX::XMFLOAT3{0.f, 0.5f, 0.f},
            DirectX::XMFLOAT3{0.f, 0.f, 0.f},
            DirectX::XMFLOAT3{5.f, 0.25f, 5.f}
        });

    m_prefabs.Spawn("StaticBox", m_world,
        {
            DirectX::XMFLOAT3{10.f, 0.7f, 0.f},
            DirectX::XMFLOAT3{0.f, 0.f, 0.f},
            DirectX::XMFLOAT3{5.f, 1.f, 5.f}
        });

    m_prefabs.Spawn("StaticBox", m_world,
        {
            DirectX::XMFLOAT3{10.f, 2.f, 0.f},
            DirectX::XMFLOAT3{0.f, 0.f, 0.f},
            DirectX::XMFLOAT3{1.f, 0.5f, 1.f}
        });

    m_prefabs.Spawn("StaticBox", m_world,
        {
            DirectX::XMFLOAT3{10.f, 2.5f, 3.f},
            DirectX::XMFLOAT3{0.f, 0.f, 0.f},
            DirectX::XMFLOAT3{1.f, 0.5f, 1.f}
        });

    m_prefabs.Spawn("StaticBox", m_world,
        {
            DirectX::XMFLOAT3{10.f, 3.f, 6.f},
            DirectX::XMFLOAT3{0.f, 0.f, 0.f},
            DirectX::XMFLOAT3{1.f, 0.5f, 1.f}
        });

    // プレイヤー：位置(0,0.5,0)、スケールは見た目用
    const EntityId playerEntity = m_prefabs.Spawn("Player", m_world,
        {
            DirectX::XMFLOAT3{0.f, 0.75f, 0.f},
            DirectX::XMFLOAT3{0.f, 180.f, 0.f},
            DirectX::XMFLOAT3{0.06f, 0.06f, 0.06f}
        });
    m_playerEntity = playerEntity;

    // --------------- Systems 構築（Update / Render 分離） ---------------
    // Update系
    m_cam = &m_sys.AddUpdate<FollowCameraSystem>();

    m_sys.AddUpdate<PlayerInputSystem>();                 // 入力 → 意図
    m_sys.AddUpdate<MovementControlSystem>(3.0f, 5.0f, 25.0f, 5.0f); // 移動制御（力/ジャンプ）
    m_sys.AddUpdate<PhysicsSystem>(-9.8f);               // 重力/外力→速度→モーション量
    m_sys.AddUpdate<CollisionSystem>(3, 0.6f);           // 衝突解決＆接地更新
    
    // Render系
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // Render系
    m_sys.AddRender<AabbDebugRenderSystem>(&m_showColliders);

    // 共有ライティング設定（起動時1回）
    const float initialCamY = 2.75f;
    const float initialCamRadius = 6.0f;
    ModelRenderSystem::ApplyDefaultLighting(initialCamY, initialCamRadius);

    // --------------- カメラエンティティ作成 ---------------
    const EntityId camEntity = m_world.Create();

    m_world.Add<ActiveCameraTag>(camEntity);

    m_world.Add<TransformComponent>(camEntity,
        DirectX::XMFLOAT3{ 0.f, 2.75f, -6.0f },
        DirectX::XMFLOAT3{ 0.f, 0.f, 0.f },
        DirectX::XMFLOAT3{ 1.f, 1.f, 1.f });

    auto& rig = m_world.Add<CameraRigComponent>(camEntity);
    rig.target = m_playerEntity;
}

/**
 * @brief SceneGame クラスのデストラクタ
 */
SceneGame::~SceneGame()
{
}

/**
 * @brief フレーム更新（入力・カメラ・ロジック）
 */
void SceneGame::Update()
{
    // ---- 入力 ----
    if (IsKeyTrigger(VK_F1))
    {
        m_showGrid = !m_showGrid;
    }
    if (IsKeyTrigger(VK_F2))
    {
        m_showColliders = !m_showColliders;
    }
    // ---- Updateパス ----
    // dt は固定でもOKだが、本来は計測値を渡す
    const float dt = 1.0f / 60.0f;
    // 修正: SystemRegistry の Tick 関数を呼び出す
    m_sys.Tick(m_world, dt);

}

/**
 * @brief フレーム描画（モデル・デバッグ）
 * @details
 * FollowCameraSystem が計算した View/Projection 行列を RenderSystem に渡し、描画を実行します。
 */
void SceneGame::Draw()
{
    SetDepthTest(true);

#ifdef _DEBUG
    // Geometory 用の View/Proj は FollowCameraSystem が毎フレーム更新済み
#endif

    // FollowCameraSystem が計算した View/Proj 行列を取得し、描画システムへ渡す
    if (m_drawModel && m_cam)
    {
        m_drawModel->SetViewProj(m_cam->GetView(), m_cam->GetProj());
    }

    // ---- Renderパス ----
    m_sys.Render(m_world);
}