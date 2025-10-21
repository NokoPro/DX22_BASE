#pragma once
#include <DirectXMath.h>
#include <cstdint>

/**
 * @file CameraRigComponent.h
 * @brief �J�����̊�{�v���p�e�B�ƒǏ]/��]�̐���p�f�[�^
 * @details
 * - �{�R���|�[�l���g�����u�J�����p�G���e�B�e�B�v��1�p�ӂ��Ďg���B
 * - �Ǐ]��itarget�j��ݒ肷��ƁAFollowCameraSystem �� Transform ���X�V�B
 * - �����b�N�͈ʒu/��]���ꂼ��ɑ΂��Čʂɐݒ�ł���B
 */
    struct CameraRigComponent
{
    // ================= ���w�n�i���e�j =================
    float fovYDeg = 60.0f;     ///< ����FOV�i�x�j
    float aspect = 16.0f / 9.0f;///< ��ʃA�X�y�N�g
    float zNear = 0.1f;      ///< �߃N���b�v
    float zFar = 1000.0f;   ///< ���N���b�v

    // ================= �Ǐ]�^�[�Q�b�g =================
    /// @note 0 �Ȃ疳���iEntityId �������̑z��j
    unsigned int target = 0;     ///< �Ǐ]��G���e�B�e�BID�iTransform �K�{�j

    // ���[���h���W�ł̃I�t�Z�b�g�i�^�[�Q�b�g��j
    DirectX::XMFLOAT3 followOffset{ 0.0f, 2.0f, -6.0f };
    // �^�[�Q�b�g�̂ǂ������邩�i�^�[�Q�b�g�ʒu + �����j
    DirectX::XMFLOAT3 lookAtOffset{ 0.0f, 1.0f, 0.0f };

    // ================= �΂�-�_���p�W���i�������j=================
    float posStiffness = 12.0f;  ///< �ʒu�̂΂ˌW���i�傫���قǑf�������j
    float posDamping = 2.0f;   ///< �ʒu�̃_���s���O�i�ՊE�ߕӂ� ~2��k �ڈ��j
    float rotStiffness = 12.0f;  ///< ��]�̂΂ˌW��
    float rotDamping = 2.0f;   ///< ��]�̃_���s���O

    // ================= �����b�N =================
    struct AxisLock {
        bool lockPosX = false, lockPosY = false, lockPosZ = false;
        bool lockRotX = false, lockRotY = false, lockRotZ = false;
    } lock;
};
