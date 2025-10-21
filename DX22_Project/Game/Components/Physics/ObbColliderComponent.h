#pragma once
#include <DirectXMath.h>

/**
 * @file ObbColliderComponent.h
 * @brief OBB�iOriented Bounding Box�j�R���C�_
 * @details
 * - Transform �̃��[���h��]�ɉ����AOBB���g�̃��[�J����]�����Ă܂��B
 * - offset �� Transform ���_����̃��[�J�����s�ړ��B
 * - halfExtents �̓��[�J�����ɉ������n�[�t�T�C�Y�B
 * - isStatic=true �͐ÓI�W�I���g���i��/�ǂȂǁj�Ƃ��Ĉ����z��B
 */
struct ObbColliderComponent
{
    DirectX::XMFLOAT3 offset{ 0.f, 0.f, 0.f };      ///< ���[�J�����S�̃I�t�Z�b�g
    DirectX::XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f }; ///< ���[�J�����̃n�[�t�T�C�Y (x,y,z)
    DirectX::XMFLOAT3 rotationDeg{ 0.f, 0.f, 0.f }; ///< OBB���[�J����]�iTransform �̉�]�ɂ���ɑ����j
    bool isStatic = false;                         ///< �ÓI�i�������ʁj���ǂ���
};
