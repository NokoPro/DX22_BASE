/**
 * @file SceneGame.cpp
 * @brief �Q�[���v���C�V�[���̎���
 * @details
 * ���̃t�@�C���ł́A�V�[���ŗL�̎��Y���[�h�AECS�V�X�e���\�z�A�G���e�B�e�B�X�|�[���A
 * ����уt���[�����Ƃ̍X�V�E�`�惍�W�b�N���������܂��B
 */
#include "SceneGame.h"
#include <cmath>
#include "Game/Prefabs/PlayerPrefab.h"
#include "Game/Prefabs/StaticBoxPrefab.h"

 // ECS�R���|�[�l���g (m_world.Add<> �̂��߂ɕK�v)
#include "../Game/Components/TransformComponent.h"
#include "../Game/Components/ModelRendererComponent.h"
#include "../Game/Components/Camera/ActiveCameraTag.h"
#include "../Game/Components/Camera/CameraRigComponent.h"

using namespace DirectX;

/**
 * @brief SceneGame �N���X�̃R���X�g���N�^
 * @details
 * ���Y���[�h�APrefab�o�^�A�����G���e�B�e�B�̃X�|�[���AECS�V�X�e���̍\�z���s���܂��B
 */
SceneGame::SceneGame()
{
    // --------------- ���Y���[�h�i���f���{�V�F�[�_�j ---------------
    const bool ok = m_mushroom.Load("Assets/Model/slime.fbx", 1.3f, Model::Flip::None);
    m_mushroom.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_mushroom.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    const bool good = m_ground.Load("Assets/Model/KayKit_Platformer_Pack_1.0_FREE/Assets/fbx/blue/platform_1x1x1_blue.fbx", 2.0f, Model::Flip::None);
    m_ground.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    m_ground.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // --------------- Prefab �o�^ ---
    // Player�i���f�����L���v�`���j
    {
        Prefabs::PlayerConfig pcfg;
        // �v���C���[�̃��f�����ƃR���W��������ݒ�
        pcfg.model = ok ? &m_mushroom : nullptr;
        pcfg.colliderHalf = { 0.4f, 0.35f, 0.4f };
        pcfg.colliderOffset = { 0.f, 0.35f, 0.f };
        pcfg.inverseMass = 1.0f;
        pcfg.linearDamping = 0.05f;
        pcfg.gravityEnabled = true;
        pcfg.gravityScale = 1.0f;

        // �C��: Prefab�֐��Ɉ��� pcfg ��n��
        m_prefabs.Register("Player", Prefabs::MakePlayerPrefab(pcfg));
    }

    // StaticBox�i��/�ǁj
    m_prefabs.Register("StaticBox", Prefabs::MakeStaticBoxPrefab(good ? &m_ground : nullptr));

    // --- �X�|�[�� ---
    // ���F���S(0,-0.25,0)�A�n�[�t(5,0.25,5)
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

    // �v���C���[�F�ʒu(0,0.5,0)�A�X�P�[���͌����ڗp
    const EntityId playerEntity = m_prefabs.Spawn("Player", m_world,
        {
            DirectX::XMFLOAT3{0.f, 0.75f, 0.f},
            DirectX::XMFLOAT3{0.f, 180.f, 0.f},
            DirectX::XMFLOAT3{0.06f, 0.06f, 0.06f}
        });
    m_playerEntity = playerEntity;

    // --------------- Systems �\�z�iUpdate / Render �����j ---------------
    // Update�n
    m_cam = &m_sys.AddUpdate<FollowCameraSystem>();

    m_sys.AddUpdate<PlayerInputSystem>();                 // ���� �� �Ӑ}
    m_sys.AddUpdate<MovementControlSystem>(3.0f, 5.0f, 25.0f, 5.0f); // �ړ�����i��/�W�����v�j
    m_sys.AddUpdate<PhysicsSystem>(-9.8f);               // �d��/�O�́����x�����[�V������
    m_sys.AddUpdate<CollisionSystem>(3, 0.6f);           // �Փˉ������ڒn�X�V
    
    // Render�n
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_sys.AddRender<DebugGridRenderSystem>(&m_showGrid);

    // Render�n
    m_sys.AddRender<AabbDebugRenderSystem>(&m_showColliders);

    // ���L���C�e�B���O�ݒ�i�N����1��j
    const float initialCamY = 2.75f;
    const float initialCamRadius = 6.0f;
    ModelRenderSystem::ApplyDefaultLighting(initialCamY, initialCamRadius);

    // --------------- �J�����G���e�B�e�B�쐬 ---------------
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
 * @brief SceneGame �N���X�̃f�X�g���N�^
 */
SceneGame::~SceneGame()
{
}

/**
 * @brief �t���[���X�V�i���́E�J�����E���W�b�N�j
 */
void SceneGame::Update()
{
    // ---- ���� ----
    if (IsKeyTrigger(VK_F1))
    {
        m_showGrid = !m_showGrid;
    }
    if (IsKeyTrigger(VK_F2))
    {
        m_showColliders = !m_showColliders;
    }
    // ---- Update�p�X ----
    // dt �͌Œ�ł�OK�����A�{���͌v���l��n��
    const float dt = 1.0f / 60.0f;
    // �C��: SystemRegistry �� Tick �֐����Ăяo��
    m_sys.Tick(m_world, dt);

}

/**
 * @brief �t���[���`��i���f���E�f�o�b�O�j
 * @details
 * FollowCameraSystem ���v�Z���� View/Projection �s��� RenderSystem �ɓn���A�`������s���܂��B
 */
void SceneGame::Draw()
{
    SetDepthTest(true);

#ifdef _DEBUG
    // Geometory �p�� View/Proj �� FollowCameraSystem �����t���[���X�V�ς�
#endif

    // FollowCameraSystem ���v�Z���� View/Proj �s����擾���A�`��V�X�e���֓n��
    if (m_drawModel && m_cam)
    {
        m_drawModel->SetViewProj(m_cam->GetView(), m_cam->GetProj());
    }

    // ---- Render�p�X ----
    m_sys.Render(m_world);
}