#pragma once
class Model; // �d���w�b�_�̈ˑ�����F�O���錾�����ɂ���

/**
 * @brief 3D���f���`��R���|�[�l���g
 * @details
 * - model �� nullptr �̏ꍇ�̓X�L�b�v
 * - �\���t���O visible �ŊȈ�ON/OFF
 * - �����I�Ƀ}�e���A���㏑��/�F��Z�Ȃǂ�ǉ����₷��
 */
struct ModelRendererComponent
{
    Model* model = nullptr;
    bool   visible = true;
};

// �����R�[�h�݊�
using ModelRenderer = ModelRendererComponent;
