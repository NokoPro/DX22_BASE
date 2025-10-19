#pragma once
#include <Windows.h>

/**
 * @file Main.h
 * @brief アプリの初期化／更新／描画／終了の外部インターフェース
 * @details
 * - WinMain から呼ばれる想定の4関数。
 * - 実体は Main.cpp に実装。
 */
HRESULT Init(HWND hWnd, UINT width, UINT height);
void    Uninit();
void    Update();
void    Draw();
