#pragma once
#include "../IRenderSystem.h"
#include <DirectXMath.h>

/**
 * @file ModelRenderSystem.h
 * @brief Transform+ModelRenderer ��`�悷�郌���_�����O�V�X�e��
 * @details
 * - View/Proj �͊O���i�J����System�j����󂯎��B
 * - ���C�e�B���O���̋��L�ݒ�͐ÓI�֐��Œ񋟁B
 */
class ModelRenderSystem final : public IRenderSystem
{
public:
    /** @brief �r���[�E�v���W�F�N�V������ݒ�i�]�u�ς݂̑z��j */
    void SetViewProj(const DirectX::XMFLOAT4X4& V, const DirectX::XMFLOAT4X4& P) { m_V = V; m_P = P; }

    /** @brief �`����s */
    void Render(const class World& world) override;  // �� Draw��Render / const World& �ɍ��킹��

    /** @brief �N�����Ɉ�x�����̃��C�e�B���O�����ݒ� */
    static void ApplyDefaultLighting(float camY, float camRadius);

private:
    DirectX::XMFLOAT4X4 m_V{}, m_P{};
};
