#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

#include "../System/Scene.h"
#include "../System/Model.h"
#include "../System/Geometory.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Input.h"
#include <DirectXMath.h>

/**
 * @brief �Q�[���{�҃V�[���i�܂��̓��f��1�̂��e�N�X�`���t���ŕ`��j
 * @details
 * - �N������ FBX/OBJ ��ǂݍ��݁iAssimp�j
 * - ShaderList �̊���V�F�[�_�iLambert�j�����蓖��
 * - ���t���[���A�J��������]�����ă��f����`��
 * - �f�o�b�O�p�ɃO���b�h��\���iF1�Ńg�O���j
 */
class SceneGame : public Scene
{
public:
    SceneGame();
    ~SceneGame();

    /** @brief �X�V�����i�J��������Ȃǁj */
    void Update() final;

    /** @brief �`�揈���i���f���{�f�o�b�O�O���b�h�j */
    void Draw() final;

private:
    /// ���f���{��
    Model m_model;

    /// �s��iworld, view, proj�j
    DirectX::XMFLOAT4X4 m_wvp[3];

    /// �J�����p�x�i�����I�[�r�b�g�p�j
    float m_camAngle = 0.0f;

    /// �f�o�b�O�O���b�h�\��
    bool m_showGrid = true;

    /// �ǂݍ��ݐ����t���O
    bool m_loaded = false;

private:
    /** @brief View/Projection �̍X�V */
    void UpdateCamera(float dt);

    /** @brief World/View/Proj �� ShaderList �ɔ��f */
    void ApplyWVP();

    /** @brief �f�t�H���g���C�e�B���O��ݒ� */
    void ApplyLighting();
};

#endif // __SCENE_GAME_H__
