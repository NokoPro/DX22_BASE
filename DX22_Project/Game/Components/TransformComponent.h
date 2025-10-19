#pragma once
#include <DirectXMath.h>

/**
 * @brief �ʒu�E��](�x)�E�X�P�[����\����{�g�����X�t�H�[��
 * @details
 * - �ˑ��� DirectXMath �̂݁i�y�ʁj
 * - ��]�͓x���@�ŕێ����A�g�p���Ƀ��W�A���֕ϊ�
 */
struct TransformComponent
{
    DirectX::XMFLOAT3 position{ 0,0,0 };
    DirectX::XMFLOAT3 rotationDeg{ 0,0,0 };
    DirectX::XMFLOAT3 scale{ 1,1,1 };
};

// �����R�[�h�݊��̂��߂̕ʖ��i���܂ł� Transform �����̂܂܎g����j
using Transform = TransformComponent;
