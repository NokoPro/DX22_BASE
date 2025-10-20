#include "SceneGame.h"
#include <cmath>
#include "Game/Prefabs/PlayerPrefab.h"
#include "Game/Prefabs/StaticBoxPrefab.h"

using namespace DirectX;

SceneGame::SceneGame()
{
    // --------------- ���Y���[�h�i���f���{�V�F�[�_�j ---------------
    const bool ok = m_mushroom.Load("Assets/Model/LowPolyNature/Mushroom_02.fbx", 1.0f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    //// --------------- ECS�F�G���e�B�e�B�쐬 ---------------
    //const EntityId e = m_world.Create();

    //auto& tr = m_world.Add<TransformComponent>(e);
    //tr.position = { 0.f, 0.f, 0.f };
    //tr.scale = { 0.06f, 0.06f, 0.06f };
    //tr.rotationDeg = { 0.f, 45.f, 0.f };

    //auto& mr = m_world.Add<ModelRendererComponent>(e);
    //mr.model = ok ? &m_mushroom : nullptr;
    //mr.visible = true;

    // --------------- Systems �\�z�iUpdate / Render �����j ---------------
    // Update�n
    m_cam = &m_sys.AddUpdate<OrbitCameraSystem>(5.f, 3.f, 60.f, 16.f / 9.f, 0.1f, 1000.f);
    m_sys.AddUpdate<PlayerInputSystem>();                 // ���� �� �Ӑ}
    m_sys.AddUpdate<MovementControlSystem>(3.0f, 5.0f, 25.0f, 5.0f); // �ړ�����i��/�W�����v�j
    m_sys.AddUpdate<PhysicsSystem>(-9.8f);               // �d��/�O�́����x�����[�V������
    m_sys.AddUpdate<CollisionSystem>(3, 0.6f);           // �Փˉ������ڒn�X�V

    // Render�n
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);


    // Render�n
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // Render�n
    m_sys.AddRender<AabbDebugRenderSystem>(&m_showColliders);

    // ���L���C�e�B���O�ݒ�i�N����1��j
    ModelRenderSystem::ApplyDefaultLighting(m_cam->CamY(), m_cam->CamRadius());

    // --- Prefab �o�^ ---
    // Player�i���f�����L���v�`���j
    {
        Prefabs::PlayerConfig pcfg;
        pcfg.model = &m_mushroom;    // ���Ƀ��[�h�ς݂̃��f��
        pcfg.colliderHalf = { 0.4f, 0.7f, 0.4f };
        pcfg.colliderOffset = { 0.f, 0.7f, 0.f };
        pcfg.inverseMass = 1.0f;
        pcfg.linearDamping = 0.05f;
        pcfg.gravityEnabled = true;
        pcfg.gravityScale = 1.0f;

        m_prefabs.Register("Player", Prefabs::MakePlayerPrefab(pcfg));
    }

    // StaticBox�i��/�ǁj
    m_prefabs.Register("StaticBox", Prefabs::MakeStaticBoxPrefab());

    // --- �X�|�[�� ---
    // ���F���S(0,-0.25,0)�A�n�[�t(5,0.25,5)
    m_prefabs.Spawn("StaticBox", m_world,
        { DirectX::XMFLOAT3{0.f, 0.5f, 0.f},
          DirectX::XMFLOAT3{0.f, 0.f, 0.f},
          DirectX::XMFLOAT3{5.f, 0.25f, 5.f} });

    // �v���C���[�F�ʒu(0,0.5,0)�A�X�P�[���͌����ڗp
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
    // ---- ���� ----
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;
    if (IsKeyTrigger(VK_F2)) m_showColliders = !m_showColliders;

    // �X�s�����x�̐ؑցi�ȈՁF�STransform�ɑ΂��Ď蓮�Ŏ~�߂�j
    if (m_pauseSpin) {
        m_world.View<TransformComponent>([&](EntityId, TransformComponent& tr) {
            (void)tr; /* �������Ȃ��iSpinSystem�͖��t���[���񂷐݌v�̂��߁A
                         �~�߂����Ȃ� SpinSystem �𑬓x0�łɍ����ւ���^�p�ł�OK�j*/
            });
    }

    // ---- Update�p�X ----
    // dt �͌Œ�ł�OK�����A�{���͌v���l��n��
    const float dt = 1.0f / 60.0f;
    m_sys.Tick(m_world, dt);
}

void SceneGame::Draw()
{
    SetDepthTest(true);

#ifdef _DEBUG
    // Geometory �p�� View/Proj �� OrbitCameraSystem �����t���[���X�V�ς�
#endif

    // �J�����s���`��V�X�e���֔z��
    if (m_drawModel && m_cam) {
        m_drawModel->SetViewProj(m_cam->GetView(), m_cam->GetProj());
    }

    // ---- Render�p�X ----
    m_sys.Render(m_world);
}
