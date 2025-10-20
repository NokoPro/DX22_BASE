#pragma once
#include <DirectXMath.h>

/**
 * @file RigidbodyComponent.h
 * @brief ���������̊j�ƂȂ�u���́v�f�[�^�i�Ȉ�/���`�̂݁j
 * @details
 * - 2D/3D���ʂ̍ŏ��v�f�B��]�͕K�v�ɂȂ�����ǉ��i�p���x/�����Ȃǁj�B
 * - ���Œ�FSemi-Implicit Euler�i���x���ʒu�̏��j��z��B
 * - �d�͂́u�����̈ꕔ�v�Ƃ��Ă����Ő���ienabled & scale�j�B
 */
struct RigidbodyComponent
{
    DirectX::XMFLOAT3 velocity{ 0.f, 0.f, 0.f }; ///< ���x (m/s)
    DirectX::XMFLOAT3 accumulatedForce{ 0.f, 0.f, 0.f }; ///< ���t���[���̊O�͍��Z�i����/��/�����o�����j

    float inverseMass = 1.0f;      ///< 1/mass�i0�Ȃ疳����̎���=�ÓI�j
    float linearDamping = 0.05f;   ///< ���x�����i0-1���x�j�B��C��R�I�Ȍ���

    bool  gravityEnabled = true;   ///< �d�͓K�p�̗L��
    float gravityScale = 1.0f;   ///< �d�͔{���i�L�������Ƃ̒����p�j
};
