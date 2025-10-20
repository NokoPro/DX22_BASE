#pragma once
#include <DirectXMath.h>

/**
 * @file CharacterControllerComponent.h
 * @brief �v���C���[/AI�́u����Ӑ}�v��\���y�ʃR���|�[�l���g
 * @details
 * - ���͂�XZ���ʂ�z��iX=���E, Z=�O��j�BY�̓W�����v�ȂǗ��U�A�N�V�����ň����B
 * - �����ł́u�������������v������ێ����A���W�⑬�x�̒��ڕύX�͂��Ȃ��B
 * - ���ۂ̕����ʁi���x/�ʒu�j�ւ̉e���� MovementControlSystem / PhysicsSystem ���s���B
 */
struct CharacterControllerComponent
{
    DirectX::XMFLOAT2 moveInput{ 0.f, 0.f }; ///< -1 - +1���x�̓��́B�����K���ŉ�
    bool jumpPressed = false;               ///< ���̃t���[���ŃW�����v�������ꂽ��
    bool runModifier = false;               ///< Shift���̑���C���i�C�Ӂj
};
