#pragma once
#include <Windows.h>

/**
 * @file Main.h
 * @brief �A�v���̏������^�X�V�^�`��^�I���̊O���C���^�[�t�F�[�X
 * @details
 * - WinMain ����Ă΂��z���4�֐��B
 * - ���̂� Main.cpp �Ɏ����B
 */
HRESULT Init(HWND hWnd, UINT width, UINT height);
void    Uninit();
void    Update();
void    Draw();
