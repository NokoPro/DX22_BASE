#include "Input.h"

//--- グローバル変数
BYTE g_keyTable[256];
BYTE g_oldTable[256];

// --- 内部状態 ---
static POINT s_prevCursor{ 0,0 };
static POINT s_deltaCursor{ 0,0 };
static int   s_wheelDelta = 0; // 120単位で積む

HRESULT InitInput()
{
    GetKeyboardState(g_keyTable);
    return S_OK;
}
void UninitInput()
{
}
void UpdateInput()
{
    memcpy_s(g_oldTable, sizeof(g_oldTable), g_keyTable, sizeof(g_keyTable));
    GetKeyboardState(g_keyTable);

    // マウス移動Δ（右ボタン押し中のみカウント）
    POINT p; GetCursorPos(&p);
    if (GetKeyState(VK_RBUTTON) & 0x8000) {
        s_deltaCursor.x = p.x - s_prevCursor.x;
        s_deltaCursor.y = p.y - s_prevCursor.y;
    }
    else {
        s_deltaCursor.x = s_deltaCursor.y = 0;
    }
    s_prevCursor = p;
}

bool IsKeyPress(BYTE key) { return g_keyTable[key] & 0x80; }
bool IsKeyTrigger(BYTE key) { return (g_keyTable[key] ^ g_oldTable[key]) & g_keyTable[key] & 0x80; }
bool IsKeyRelease(BYTE key) { return (g_keyTable[key] ^ g_oldTable[key]) & g_oldTable[key] & 0x80; }
bool IsKeyRepeat(BYTE) { return false; }

// --- 新規追加 ---
bool  IsMouseDownR() { return (GetKeyState(VK_RBUTTON) & 0x8000) != 0; }
POINT GetMouseDelta() { return s_deltaCursor; }

int ConsumeMouseWheelDelta() 
{
    int v = s_wheelDelta;
    s_wheelDelta = 0;
    return v;
}

void Input_OnMouseWheel(short delta)
{
    s_wheelDelta += (int)delta; // 120/ノッチ
}
