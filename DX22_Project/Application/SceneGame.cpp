#include "SceneGame.h"
#include <cmath>

/** @brief �p�x�̖��b��]���x�i���W�A���j */
static constexpr float kCamAngularSpeed = 0.4f;
/** @brief �J�������a */
static constexpr float kCamRadius = 5.0f;
/** @brief �J�������� */
static constexpr float kCamHeight = 3.0f;
/** @brief ��������p�i�x�j */
static constexpr float kFovYDeg = 60.0f;
/** @brief ��/���N���b�v */
static constexpr float kZNear = 0.1f, kZFar = 1000.0f;

SceneGame::SceneGame()
{
    // �s�񏉊���
    for (int i = 0; i < 3; ++i)
        DirectX::XMStoreFloat4x4(&m_wvp[i], DirectX::XMMatrixIdentity());

    // ---- ���f���Ǎ� ----
    // ��jAssets/Model/LowPolyNature/Mushroom_02.fbx
    // �� ���s�t�@�C������̑��΃p�X�ɂȂ�̂ŁA���@�p�X�����킹�Ă��������B
    //    ���܂������Ȃ����̓t���p�X�ň�x�m�F���������߁B
    m_loaded = m_model.Load("Assets/Model/LowPolyNature/Mushroom_01.fbx",
        /*scale*/ 0.02f,
        Model::Flip::None);

    // ---- �g���V�F�[�_�������ւ��iShaderList �̊���Q�j----
    // �X�L�j���O�s�v�̒ʏ탁�b�V���� VS_WORLD ���g��
    m_model.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    // �����o�[�g�g�U�{�e�N�X�`���̕W�� PS
    m_model.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // �����J�����K�p�����C�e�B���O
    UpdateCamera(0.0f);
    ApplyWVP();
    ApplyLighting();
}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
    // ���͂ŃO���b�h�\���ؑ�
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;

    // �J����������]�iEnter �Œ�~/�ĊJ�������ꍇ�̓g�O���ǉ����j
    // �Ƃ肠�����펞�񂷁Fdt �͂Ƃ�Â炢�̂ŌŒ荏��
    UpdateCamera(1.0f / 60.0f);
    ApplyWVP();
}

void SceneGame::Draw()
{
    // 3D�`��ł͐[�x�e�X�gON����
    SetDepthTest(true);

#ifdef _DEBUG
    if (m_showGrid)
    {
        // �f�o�b�O�O���b�h�iXZ���ʁj
        const DirectX::XMFLOAT4 gridCol(0.5f, 0.5f, 0.5f, 1.0f);
        const float size = 20.0f;
        const float step = 1.0f;
        for (float g = -size; g <= size; g += step)
        {
            Geometory::AddLine({ g, 0, -size }, { g, 0,  size }, gridCol);
            Geometory::AddLine({ -size, 0, g }, { size, 0, g }, gridCol);
        }
        // ��
        Geometory::AddLine({ 0,0,0 }, { 3,0,0 }, { 1,0,0,1 });
        Geometory::AddLine({ 0,0,0 }, { 0,3,0 }, { 0,1,0,1 });
        Geometory::AddLine({ 0,0,0 }, { 0,0,3 }, { 0,0,1,1 });
        Geometory::DrawLines();
    }
#endif

    if (m_loaded)
    {
        // �K�v�Ȃ炱���Ń��[���h�s����X�V�i��]�Ō��������ꍇ�Ȃǁj
        DirectX::XMMATRIX W =
            DirectX::XMMatrixRotationY(0.0f) *
            DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&m_wvp[0], DirectX::XMMatrixTranspose(W));
        ApplyWVP();

        // �}�e���A�����̃e�N�X�`���� Model::Draw �������o�C���h���Ă����
        // �i�}�e���A���ǂݍ��ݎ��� pTexture ���Z�b�g�����j
        m_model.Draw();
    }

    // �K�v�ɉ����ăX�v���C�g��UI�������ŕ`��
}

void SceneGame::UpdateCamera(float dt)
{
    m_camAngle += kCamAngularSpeed * dt;

    const float cx = std::cos(m_camAngle) * kCamRadius;
    const float cz = std::sin(m_camAngle) * kCamRadius;
    const DirectX::XMVECTOR eye = DirectX::XMVectorSet(cx, kCamHeight, cz, 0.0f);
    const DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.8f, 0.0f, 0.0f);
    const DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    const DirectX::XMMATRIX V =
        DirectX::XMMatrixLookAtLH(eye, at, up);
    const DirectX::XMMATRIX P =
        DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(kFovYDeg),
            16.0f / 9.0f,  // ��ʔ䗦�B�K�v�Ȃ���E�B���h�E�T�C�Y�ɍ��킹�Ă�������
            kZNear, kZFar);

    DirectX::XMStoreFloat4x4(&m_wvp[1], DirectX::XMMatrixTranspose(V));
    DirectX::XMStoreFloat4x4(&m_wvp[2], DirectX::XMMatrixTranspose(P));

    // Geometory ���̃J�����ɂ����f�i�f�o�b�O���C���p�j
    Geometory::SetView(m_wvp[1]);
    Geometory::SetProjection(m_wvp[2]);
}

void SceneGame::ApplyWVP()
{
    // ShaderList �� VS �� world/view/proj ���ꊇ�ŋl�߂�
    ShaderList::SetWVP(m_wvp);
}

void SceneGame::ApplyLighting()
{
    // �K���ȕ��s�����ƃJ�����ʒu��ݒ�iLambert/Specular/Toon �ŋ��L�j
    ShaderList::SetLight({ 1,1,1,1 }, { -1,-1,-1 });
    ShaderList::SetCameraPos({ 0.0f, kCamHeight, -kCamRadius });
    // �t�H�O���s�v�Ȃ�ݒ肵�Ȃ��Ă�OK�iPS_FOG�̂ݎg�p�j
}
