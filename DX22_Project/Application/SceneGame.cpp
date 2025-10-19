#include "SceneGame.h"
#include <cmath>

using namespace DirectX;

SceneGame::SceneGame()
{
    // --------------- 資産ロード（モデル＋シェーダ） ---------------
    const bool ok = m_mushroom.Load("Assets/Model/LowPolyNature/Mushroom_02.fbx", 1.0f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // --------------- ECS：エンティティ作成 ---------------
    const EntityId e = m_world.Create();

    auto& tr = m_world.Add<TransformComponent>(e);
    tr.position = { 0.f, 0.f, 0.f };
    tr.scale = { 0.06f, 0.06f, 0.06f };
    tr.rotationDeg = { 0.f, 45.f, 0.f };

    auto& mr = m_world.Add<ModelRendererComponent>(e);
    mr.model = ok ? &m_mushroom : nullptr;
    mr.visible = true;

    // --------------- Systems 構築（Update / Render 分離） ---------------
    // Update系
    m_cam = &m_sys.AddUpdate<OrbitCameraSystem>(5.f, 3.f, 60.f, 16.f / 9.f, 0.1f, 1000.f);
    // 回転デモ（F2で停止/再開したいので保持しない：毎フレーム Add/Remove ではなく、速度0にする）
    auto& spin = m_sys.AddUpdate<SpinSystem>(45.f); (void)spin;

    // Render系
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // 共有ライティング設定（起動時1回）
    ModelRenderSystem::ApplyDefaultLighting(m_cam->CamY(), m_cam->CamRadius());
}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
    // ---- 入力 ----
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;
    if (IsKeyTrigger(VK_F2)) m_pauseSpin = !m_pauseSpin;

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
