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

// ホイールの増分を取得して 0 にリセット
int  ConsumeMouseWheelDelta();

// WndProc から呼ぶ用：ホイールの生Δを積む（120 単位）
void Input_OnMouseWheel(short delta);
#endif // __INPUT_H__