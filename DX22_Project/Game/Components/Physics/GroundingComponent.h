#pragma once
#include <DirectXMath.h>

/**
 * @file GroundingComponent.h
 * @brief �ڒn��Ԃ̃L���b�V���i�Փˉ������X�V����j
 * @details
 * - CollisionSystem ���u�ڐG�@����������Ƃ݂Ȃ��邩�v�𔻒肵�čX�V�B
 * - ����/�ړ����͂��̃t���O�ŃW�����v���␂�����x�̃N�����v���s���B
 */
struct GroundingComponent
{
    bool grounded = false;                          ///< �ڒn���Ă��邩
    DirectX::XMFLOAT3 groundNormal{ 0.f, 1.f, 0.f };  ///< ���߂̐ڒn�@��
    float timeSinceGrounded = 1e9f;                 ///< �Ō�ɐڒn���Ă���̕b��

    bool hasContact = false;
    DirectX::XMFLOAT3 contactNormal{ 0,0,0 };
};
