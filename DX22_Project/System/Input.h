#ifndef __INPUT_H__
#define __INPUT_H__

#include <Windows.h>
#undef max
#undef min

HRESULT InitInput();
void UninitInput();
void UpdateInput();

bool IsKeyPress(BYTE key);
bool IsKeyTrigger(BYTE key);
bool IsKeyRelease(BYTE key);
bool IsKeyRepeat(BYTE key);

bool IsMouseDownR();
POINT GetMouseDelta();

// �z�C�[���̑������擾���� 0 �Ƀ��Z�b�g
int  ConsumeMouseWheelDelta();

// WndProc ����Ăԗp�F�z�C�[���̐�����ςށi120 �P�ʁj
void Input_OnMouseWheel(short delta);
#endif // __INPUT_H__