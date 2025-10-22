#pragma once
#include <DirectXMath.h>
#include "../../ECS.h"

/**
 * @file CameraRigComponent.h
 * @brief �J���������i�Ǐ]�^�I�[�r�b�g�j�̒����p�����[�^��ێ�����R���|�[�l���g
 * @details
 * - �^�[�Q�b�g�iEntityId�j����ɁA�����_�ilookAtOffset�j�փJ�����������܂��B
 * - orbitEnabled=true �̂Ƃ��A�^�[�Q�b�g���x�_�� yaw/pitch �ŉ�]���Adistance ��ۂ��܂��B
 * - posStiffness/posDamping �͒Ǐ]�̃o�l�E�_���p�W���ł��i�X���[�Y���̒����j�B
 */
struct CameraRigComponent
{
    /** @name �^�[�Q�b�g/��p */
    ///@{
    EntityId target = 0;                 ///< �Ǐ]/��������ΏۃG���e�B�e�B
    float    fovYDeg = 60.f;             ///< ��������p�i�x�j
    float    aspect = 16.f / 9.f;         ///< �A�X�y�N�g��
    float    zNear = 0.1f;             ///< �߃N���b�v
    float    zFar = 100.f;            ///< ���N���b�v
    ///@}

    /** @name �Ǐ]�p�����[�^ */
    ///@{
    DirectX::XMFLOAT3 followOffset{ 0, 2.0f, -4.0f }; ///< �Ǐ]���̑��΃I�t�Z�b�g
    DirectX::XMFLOAT3 lookAtOffset{ 0, 1.0f,  0.0f }; ///< �����_�I�t�Z�b�g�i�^�[�Q�b�g���W�n�j
    float posStiffness = 28.f;                       ///< �Ǐ]�o�l�W���i�傫���قǑf�������j
    float posDamping = 10.f;                       ///< �Ǐ]�_���p�W���i�傫���قǌ������j
    struct { bool lockPosX = false, lockPosY = false, lockPosZ = false; } lock{}; ///< �����b�N
    ///@}

    /** @name �I�[�r�b�g�i�^�[�Q�b�g�x�_��]�j */
    ///@{
    bool  orbitEnabled = true;    ///< true �ŃI�[�r�b�g�L��
    float orbitYawDeg = 180.f;  ///< �����p�i�x�j
    float orbitPitchDeg = 15.f;   ///< ��p�i�x�j
    float orbitMinPitch = -80.f;  ///< ��p�̉���
    float orbitMaxPitch = 80.f;  ///< ��p�̏��
    float orbitDistance = 4.0f;   ///< �^�[�Q�b�g����̋���
    float orbitMinDist = 1.2f;   ///< �����̉���
    float orbitMaxDist = 12.0f;  ///< �����̏��
    ///@}

    /** @name ���͊��x */
    ///@{
    float sensYaw = 0.15f;  ///< Yaw ���x�ideg / ����1.0�j
    float sensPitch = 0.12f;  ///< Pitch ���x�ideg / ����1.0�j
    float sensZoom = 0.25f;  ///< �Y�[�����x�im / �X�e�b�v�j
    ///@}
};
