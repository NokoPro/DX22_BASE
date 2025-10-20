#include "SceneGame.h"
#include <cmath>
#include "Game/Prefabs/PlayerPrefab.h"
#include "Game/Prefabs/StaticBoxPrefab.h"

using namespace DirectX;

SceneGame::SceneGame()
{
    // --------------- 資産ロード（モデル＋シェーダ） ---------------
    const bool ok = m_mushroom.Load("Assets/Model/LowPolyNature/Mushroom_02.fbx", 1.0f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    //// --------------- ECS：エンティティ作成 ---------------
    //const EntityId e = m_world.Create();

    //auto& tr = m_world.Add<TransformComponent>(e);
    //tr.position = { 0.f, 0.f, 0.f };
    //tr.scale = { 0.06f, 0.06f, 0.06f };
    //tr.rotationDeg = { 0.f, 45.f, 0.f };

    //auto& mr = m_world.Add<ModelRendererComponent>(e);
    //mr.model = ok ? &m_mushroom : nullptr;
    //mr.visible = true;

    // --------------- Systems 構築（Update / Render 分離） ---------------
    // Update系
    m_cam = &m_sys.AddUpdate<OrbitCameraSystem>(5.f, 3.f, 60.f, 16.f / 9.f, 0.1f, 1000.f);
    m_sys.AddUpdate<PlayerInputSystem>();                 // 入力 → 意図
    m_sys.AddUpdate<MovementControlSystem>(3.0f, 5.0f, 25.0f, 5.0f); // 移動制御（力/ジャンプ）
    m_sys.AddUpdate<PhysicsSystem>(-9.8f);               // 重力/外力→速度→モーション量
    m_sys.AddUpdate<CollisionSystem>(3, 0.6f);           // 衝突解決＆接地更新

    // Render系
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);


    // Render系
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // Render系
    m_sys.AddRender<AabbDebugRenderSystem>(&m_showColliders);

    // 共有ライティング設定（起動時1回）
    ModelRenderSystem::ApplyDefaultLighting(m_cam->CamY(), m_cam->CamRadius());

    // --- Prefab 登録 ---
    // Player（モデルをキャプチャ）
    {
        Prefabs::PlayerConfig pcfg;
        pcfg.model = &m_mushroom;    // 既にロード済みのモデル
        pcfg.colliderHalf = { 0.4f, 0.7f, 0.4f };
        pcfg.colliderOffset = { 0.f, 0.7f, 0.f };
        pcfg.inverseMass = 1.0f;
        pcfg.linearDamping = 0.05f;
        pcfg.gravityEnabled = true;
        pcfg.gravityScale = 1.0f;

        m_prefabs.Register("Player", Prefabs::MakePlayerPrefab(pcfg));
    }

    // StaticBox（床/壁）
    m_prefabs.Register("StaticBox", Prefabs::MakeStaticBoxPrefab());

    // --- スポーン ---
    // 床：中心(0,-0.25,0)、ハーフ(5,0.25,5)
    m_prefabs.Spawn("StaticBox", m_world,
        { DirectX::XMFLOAT3{0.f, 0.5f, 0.f},
          DirectX::XMFLOAT3{0.f, 0.f, 0.f},
          DirectX::XMFLOAT3{5.f, 0.25f, 5.f} });

    // プレイヤー：位置(0,0.5,0)、スケールは見た目用
    m_prefabs.Spawn("Player", m_world,
        { DirectX::XMFLOAT3{0.f, 0.75f, 0.f},
          DirectX::XMFLOAT3{0.f, 0.f, 0.f},
          DirectX::XMFLOAT3{0.06f, 0.06f, 0.06f} });

}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
    // ---- 入力 ----
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;
    if (IsKeyTrigger(VK_F2)) m_showColliders = !m_showColliders;

    // スピン速度の切替（簡易：全Transformに対して手動で止める）
    if (m_pauseSpin) {
        m_world.View<TransformComponent>([&](EntityId, TransformComponent& tr) {
            (void)tr; /* 何もしない（SpinSystemは毎フレーム回す設計のため、
                         止めたいなら SpinSystem を速度0版に差し替える運用でもOK）*/
            });
    }

    // ---- Updateパス ----
    // dt は固定でもOKだが、本来は計測値を渡す
    const float dt = 1.0f / 60.0f;
    m_sys.Tick(m_world, dt);
}

void SceneGame::Draw()
{
    SetDepthTest(true);

#ifdef _DEBUG
    // Geometory 用の View/Proj は OrbitCameraSystem が毎フレーム更新済み
#endif

    // カメラ行列を描画システムへ配線
    if (m_drawModel && m_cam) {
        m_drawModel->SetViewProj(m_cam->GetView(), m_cam->GetProj());
    }

    // ---- Renderパス ----
    m_sys.Render(m_world);
}
