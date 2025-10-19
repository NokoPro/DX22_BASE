/*****************************************************************//**
 * @file   Main.cpp
 * @brief  ECSを用いた3Dベースプログラム
 * 
 * @author 浅野勇生
 * @date   2025/10/20 - 制作開始、ECSの組み込み
 *********************************************************************/
#include "Main.h"
#include <memory>
#include <crtdbg.h>
#include <windows.h>
#include <mmsystem.h> // timeGetTime, timeBeginPeriod
#pragma comment(lib, "winmm.lib")

// ==== Engine/Framework ====
#include "../System/Defines.h"                 // SCREEN_WIDTH/HEIGHT, APP_TITLE, fFPS
#include "../System/DirectX/DirectX.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Geometory.h"
#include "../System/Sprite.h"
#include "../System/Input.h"
#include "../System/Scene.h"

// ==== Game ====
#include "SceneManager.h"    // ★新規：単一アクティブシーンの管理
#include "SceneGame.h"       // 最初に起動するゲーム本編シーン

//-----------------------------------------------------------------------------
// グローバル（この翻訳単位内限定）
//-----------------------------------------------------------------------------
namespace {
    std::unique_ptr<SceneManager> g_scene;   ///< 現在アクティブなシーンを保持
}

//-----------------------------------------------------------------------------
// プロトタイプ
//-----------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//-----------------------------------------------------------------------------
/**
 * @brief アプリのエントリポイント
 * @details
 * - ウィンドウ生成 → フレームループ（メッセージ＆ゲーム処理） → 終了
 * - 60FPS目標の簡易ウェイト付き。必要なら可変FPS/固定タイムステップに拡張可。
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // メモリリーク検知（Debug）
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // ---- ウィンドウクラス登録 ----
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "DX11_ECS_Class";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, "RegisterClassEx failed", "Error", MB_OK);
        return 0;
    }

    // ---- ウィンドウ作成 ----
    RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    const DWORD style = WS_CAPTION | WS_SYSMENU;
    const DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    HWND hWnd = CreateWindowEx(
        exStyle, wc.lpszClassName, APP_TITLE, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInst, nullptr
    );
    if (!hWnd) {
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // ---- フレームワーク初期化 ----
    if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT))) {
        Uninit();
        DestroyWindow(hWnd);
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    // ---- 目標FPS制御のためのタイマ設定 ----
    timeBeginPeriod(1); // 1ms精度（OS依存）

    const double targetMs = 1000.0 / static_cast<double>(fFPS);
    DWORD lastTick = timeGetTime();

    // ---- メインループ（メッセージ優先＋アイドルでゲーム処理）----
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    for (;;)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // 時間計測（簡易：固定フレーム境界で更新・描画）
        const DWORD now = timeGetTime();
        const double elapsed = static_cast<double>(now - lastTick);

        if (elapsed >= targetMs)
        {
            // 1フレーム分の処理
            Update();
            Draw();
            lastTick = now;
        }
        else
        {
            // 余剰時間は軽くスリープ（CPU占有を避ける）
            const DWORD sleepMs = static_cast<DWORD>(targetMs - elapsed);
            if (sleepMs > 0) Sleep(sleepMs);
        }
    }

    timeEndPeriod(1);

    // ---- 終了処理 ----
    Uninit();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, hInst);
    return static_cast<int>(msg.wParam);
}

//-----------------------------------------------------------------------------
/**
 * @brief ウィンドウプロシージャ
 * @details 必要に応じてキー入力やリサイズ等を追加。
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

//-----------------------------------------------------------------------------
/**
 * @brief フレームワーク／サブシステム初期化
 * @param hWnd   作成済みウィンドウハンドル
 * @param width  画面幅
 * @param height 画面高さ
 */
HRESULT Init(HWND hWnd, UINT width, UINT height)
{
    // DirectX
    if (FAILED(InitDirectX(hWnd, width, height, /*fullscreen=*/false))) return E_FAIL;

    // Subsystems
    if (FAILED(InitInput())) return E_FAIL;
    ShaderList::Init();
    Geometory::Init();
    Sprite::Init();

    // Scene: 最初のシーンを構築
    g_scene.reset(new SceneManager());
    g_scene->Change<SceneGame>();  // ★ECSベースの本編シーンに切替

    return S_OK;
}

//-----------------------------------------------------------------------------
/**
 * @brief フレームワーク終了処理（逆順で破棄）
 */
void Uninit()
{
    g_scene.reset();

    Sprite::Uninit();
    Geometory::Uninit();
    ShaderList::Uninit();
    UninitInput();
    UninitDirectX();
}

//-----------------------------------------------------------------------------
/**
 * @brief 1フレーム分の更新処理
 * @details
 * - 入力の更新 → アクティブシーンの Update。
 * - SceneManager は常に「1シーンのみ」アクティブ。
 */
void Update()
{
    UpdateInput();
    if (g_scene) g_scene->Update();  // ★以前の g_pScene->RootUpdate() を置換
}

//-----------------------------------------------------------------------------
/**
 * @brief 1フレーム分の描画処理
 * @details
 * - Begin/End でフレームを囲み、SceneManager 経由で描画。
 */
void Draw()
{
    BeginDrawDirectX();

    if (g_scene) g_scene->Draw();    // ★以前の g_pScene->RootDraw() を置換

    EndDrawDirectX();
}
