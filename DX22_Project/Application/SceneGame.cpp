#include "SceneGame.h"
#include <cmath>

using namespace DirectX;

SceneGame::SceneGame()
{
    // --------------- ���Y���[�h�i���f���{�V�F�[�_�j ---------------
    const bool ok = m_mushroom.Load("Assets/Model/LowPolyNature/Mushroom_02.fbx", 1.0f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // --------------- ECS�F�G���e�B�e�B�쐬 ---------------
    const EntityId e = m_world.Create();

    auto& tr = m_world.Add<TransformComponent>(e);
    tr.position = { 0.f, 0.f, 0.f };
    tr.scale = { 0.06f, 0.06f, 0.06f };
    tr.rotationDeg = { 0.f, 45.f, 0.f };

    auto& mr = m_world.Add<ModelRendererComponent>(e);
    mr.model = ok ? &m_mushroom : nullptr;
    mr.visible = true;

    // --------------- Systems �\�z�iUpdate / Render �����j ---------------
    // Update�n
    m_cam = &m_sys.AddUpdate<OrbitCameraSystem>(5.f, 3.f, 60.f, 16.f / 9.f, 0.1f, 1000.f);
    // ��]�f���iF2�Œ�~/�ĊJ�������̂ŕێ����Ȃ��F���t���[�� Add/Remove �ł͂Ȃ��A���x0�ɂ���j
    auto& spin = m_sys.AddUpdate<SpinSystem>(45.f); (void)spin;

    // Render�n
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // ���L���C�e�B���O�ݒ�i�N����1��j
    ModelRenderSystem::ApplyDefaultLighting(m_cam->CamY(), m_cam->CamRadius());
}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
    // ---- ���� ----
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;
    if (IsKeyTrigger(VK_F2)) m_pauseSpin = !m_pauseSpin;

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
