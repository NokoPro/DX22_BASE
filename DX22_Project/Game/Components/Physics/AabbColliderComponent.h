#pragma once
#include <DirectXMath.h>

/**
 * @file AabbColliderComponent.h
 * @brief AABB�i�����s�o�E���f�B���O�{�b�N�X�j�R���C�_
 * @details
 * - Transform�i���[���h���W�j�̏�Ɂu�I�t�Z�b�g+�n�[�t�T�C�Y�v�Ŕz�u����݌v�B
 * - isStatic=true �̂��͎̂��ʖ��������Ƃ��ē������Ȃ��i�n��/�ǂȂǁj�B
 * - �}�e���A���┽��/���C�͕K�v�ɉ����ʃR���|�[�l���g�Ŋg���B
 */
struct AabbColliderComponent
{
    DirectX::XMFLOAT3 offset{ 0.f, 0.f, 0.f };      ///< Transform���_����̃I�t�Z�b�g
    DirectX::XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f }; ///< �n�[�t�T�C�Y�ix,y,z�j
    bool isStatic = false;                         ///< �ÓI�i�n��/�ǁj���ǂ���
};
