#pragma once
#include "../World.h"  // World���g���Ȃ�K�v�B�O���錾�ł��B

/**
 * @class IUpdateSystem
 * @brief ���t���[���̍X�V������S������V�X�e���̊��N���X
 * @details
 * ECS�ɂ�����uSystem�v�̂����A**���W�b�N�X�V�i�����E���́E�A�j���[�V�����Ȃǁj**
 * ���s���N���X���p�����܂��B
 *
 * ���ׂĂ̔h���N���X�� Update() ���������A
 * World ���̃R���|�[�l���g�Q���Q�ƁE�ύX���܂��B
 */
class IUpdateSystem
{
public:
    /// @brief ���z�f�X�g���N�^�i�h���N���X�j���������S�j
    virtual ~IUpdateSystem() = default;

    /**
     * @brief �X�V����
     * @param[in,out] world ECS���[���h�S�̂ւ̎Q��
     * @param[in] dt �O�t���[������̌o�ߎ��ԁi�b�j
     * @note �������z�֐��B�h���N���X�ŕK����������K�v������܂��B
     */
    virtual void Update(class World& world, float dt) = 0;
};