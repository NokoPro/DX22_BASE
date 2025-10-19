#pragma once
#include "../World.h"

/**
 * @class IRenderSystem
 * @brief �`�揈����S������V�X�e���̊��N���X
 * @details
 * ECS�ɂ�����uSystem�v�̂����A**�`��i���f���A�X�v���C�g�A�f�o�b�O���Ȃǁj**
 * ���s���N���X���p�����܂��B
 *
 * Update �n�Ƃ͕������A�`�揇���𐧌䂵�₷�����܂��B
 */
class IRenderSystem
{
public:
    /// @brief ���z�f�X�g���N�^�i�h���N���X�j���������S�j
    virtual ~IRenderSystem() = default;

    /**
     * @brief �`�揈��
     * @param[in] world ECS���[���h�S�̂ւ̎Q��
     * @note �������z�֐��B�h���N���X�ŕK����������K�v������܂��B
     */
    virtual void Render(const class World& world) = 0;
};
