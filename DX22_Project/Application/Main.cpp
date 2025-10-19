/*****************************************************************//**
 * @file   Main.cpp
 * @brief  ECS��p����3D�x�[�X�v���O����
 * 
 * @author ���E��
 * @date   2025/10/20 - ����J�n�AECS�̑g�ݍ���
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
#include "SceneManager.h"    // ���V�K�F�P��A�N�e�B�u�V�[���̊Ǘ�
#include "SceneGame.h"       // �ŏ��ɋN������Q�[���{�҃V�[��

//-----------------------------------------------------------------------------
// �O���[�o���i���̖|��P�ʓ�����j
//-----------------------------------------------------------------------------
namespace {
    std::unique_ptr<SceneManager> g_scene;   ///< ���݃A�N�e�B�u�ȃV�[����ێ�
}

//-----------------------------------------------------------------------------
// �v���g�^�C�v
//-----------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//-----------------------------------------------------------------------------
/**
 * @brief �A�v���̃G���g���|�C���g
 * @details
 * - �E�B���h�E���� �� �t���[�����[�v�i���b�Z�[�W���Q�[�������j �� �I��
 * - 60FPS�ڕW�̊ȈՃE�F�C�g�t���B�K�v�Ȃ��FPS/�Œ�^�C���X�e�b�v�Ɋg���B
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // ���������[�N���m�iDebug�j
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // ---- �E�B���h�E�N���X�o�^ ----
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

    // ---- �E�B���h�E�쐬 ----
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

    // ---- �t���[�����[�N������ ----
    if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT))) {
        Uninit();
        DestroyWindow(hWnd);
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    // ---- �ڕWFPS����̂��߂̃^�C�}�ݒ� ----
    timeBeginPeriod(1); // 1ms���x�iOS�ˑ��j

    const double targetMs = 1000.0 / static_cast<double>(fFPS);
    DWORD lastTick = timeGetTime();

    // ---- ���C�����[�v�i���b�Z�[�W�D��{�A�C�h���ŃQ�[�������j----
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

        // ���Ԍv���i�ȈՁF�Œ�t���[�����E�ōX�V�E�`��j
        const DWORD now = timeGetTime();
        const double elapsed = static_cast<double>(now - lastTick);

        if (elapsed >= targetMs)
        {
            // 1�t���[�����̏���
            Update();
            Draw();
            lastTick = now;
        }
        else
        {
            // �]�莞�Ԃ͌y���X���[�v�iCPU��L�������j
            const DWORD sleepMs = static_cast<DWORD>(targetMs - elapsed);
            if (sleepMs > 0) Sleep(sleepMs);
        }
    }

    timeEndPeriod(1);

    // ---- �I������ ----
    Uninit();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, hInst);
    return static_cast<int>(msg.wParam);
}

//-----------------------------------------------------------------------------
/**
 * @brief �E�B���h�E�v���V�[�W��
 * @details �K�v�ɉ����ăL�[���͂⃊�T�C�Y����ǉ��B
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
 * @brief �t���[�����[�N�^�T�u�V�X�e��������
 * @param hWnd   �쐬�ς݃E�B���h�E�n���h��
 * @param width  ��ʕ�
 * @param height ��ʍ���
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

    // Scene: �ŏ��̃V�[�����\�z
    g_scene.reset(new SceneManager());
    g_scene->Change<SceneGame>();  // ��ECS�x�[�X�̖{�҃V�[���ɐؑ�

    return S_OK;
}

//-----------------------------------------------------------------------------
/**
 * @brief �t���[�����[�N�I�������i�t���Ŕj���j
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
 * @brief 1�t���[�����̍X�V����
 * @details
 * - ���͂̍X�V �� �A�N�e�B�u�V�[���� Update�B
 * - SceneManager �͏�Ɂu1�V�[���̂݁v�A�N�e�B�u�B
 */
void Update()
{
    UpdateInput();
    if (g_scene) g_scene->Update();  // ���ȑO�� g_pScene->RootUpdate() ��u��
}

//-----------------------------------------------------------------------------
/**
 * @brief 1�t���[�����̕`�揈��
 * @details
 * - Begin/End �Ńt���[�����͂݁ASceneManager �o�R�ŕ`��B
 */
void Draw()
{
    BeginDrawDirectX();

    if (g_scene) g_scene->Draw();    // ���ȑO�� g_pScene->RootDraw() ��u��

    EndDrawDirectX();
}
