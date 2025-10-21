#pragma once
#include "../IUpdateSystem.h"
#include <DirectXMath.h>

/**
 * @file FollowCameraSystem.h
 * @brief �^�[�Q�b�g�Ǐ] + �����b�N + �X���[�Y�Ǐ]���s���J�����X�V�V�X�e��
 * @details
 * - ActiveCameraTag + CameraRigComponent + Transform �����G���e�B�e�B��Ώۂɓ���B
 * - �Ǐ]�� target �� Transform ���Q�Ƃ��A�J������ Transform ���X�V�B
 * - View/Projection �s��𐶐����āA�O���֒񋟁iModelRenderSystem �Ȃǂ֔z���j�B
 * - Geometory �� View/Proj �������ōX�V���Ă����ƃf�o�b�O�`�悪��������B
 */
class FollowCameraSystem final : public IUpdateSystem {
public:
    FollowCameraSystem() = default;

    /// @brief 1�t���[�����̍X�V�i�Ǐ]/�X���[�W���O/�s��X�V�j
    void Update(class World& world, float dt) override;

    /// @brief �]�u�ς݂� View �s����擾�i�`��p�j
    const DirectX::XMFLOAT4X4& GetView() const { return m_V; }

    /// @brief �]�u�ς݂� Projection �s����擾�i�`��p�j
    const DirectX::XMFLOAT4X4& GetProj() const { return m_P; }

private:
    DirectX::XMFLOAT4X4 m_V{}; ///< �]�u�ς� View
    DirectX::XMFLOAT4X4 m_P{}; ///< �]�u�ς� Projection
};
