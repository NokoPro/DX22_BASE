#pragma once
#include <DirectXMath.h>

/**
 * @file MotionDeltaComponent.h
 * @brief ���t���[���Łu�ړ��������ʁv��\���ꎞ�o�b�t�@
 * @details
 * - PhysicsSystem �����x����Z�o���ď������݁ACollisionSystem �������
 *   �g���ăX�C�[�v/���U�������s���ATransform�𐳂��B
 * - �Փˌ�ACollisionSystem ������ăN���A����i�܂��͏㏑������j�B
 */
struct MotionDeltaComponent
{
    DirectX::XMFLOAT3 delta{ 0.f, 0.f, 0.f }; ///< �t���[�����̊�]�ړ��ʁim�j
};
