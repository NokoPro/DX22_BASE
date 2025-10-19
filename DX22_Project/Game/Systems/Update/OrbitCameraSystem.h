#pragma once
#include "../IUpdateSystem.h"
#include <DirectXMath.h>

/**
 * @file OrbitCameraSystem.h
 * @brief �J���������񂳂���X�V�V�X�e���iView/Proj��ێ��j
 * @details
 * - �}�E�X���쓙����ꂽ���ꍇ�������Ɏ����B
 * - �����_�����O���ւ� GetView()/GetProj() �ōs��񋟁B
 */
class OrbitCameraSystem final : public IUpdateSystem 
{
public:
    /**
     * @brief �R���X�g���N�^
     * @param radius ���񔼌a
     * @param height �J��������
     * @param fovYDeg ����FOV�i�x�j
     * @param aspect ��ʃA�X�y�N�g
     * @param zNear �߃N���b�v
     * @param zFar ���N���b�v
     */
    OrbitCameraSystem(float radius = 5.f, float height = 3.f,
        float fovYDeg = 60.f, float aspect = 16.f / 9.f,
        float zNear = 0.1f, float zFar = 1000.f);

    void Update(class World& world, float dt) override;

    /** @brief View�s��i�]�u�ς݁j�擾 */
    const DirectX::XMFLOAT4X4& GetView() const { return m_V; }

    /** @brief Proj�s��i�]�u�ς݁j�擾 */
    const DirectX::XMFLOAT4X4& GetProj() const { return m_P; }

    /** @brief �J���������̎擾�i���C�e�B���O�ݒ�ȂǂŎg�p�j */
    float CamY() const { return m_height; }

    /** @brief �J�������a�̎擾�i���C�e�B���O�ݒ�ȂǂŎg�p�j */
    float CamRadius() const { return m_radius; }

private:
    float m_angle = 0.f;
    float m_radius, m_height, m_fovYDeg, m_aspect, m_zNear, m_zFar;
    DirectX::XMFLOAT4X4 m_V{}, m_P{};
};
