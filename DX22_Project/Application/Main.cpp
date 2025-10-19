#include "Main.h"
#include <memory>
#include "../System/DirectX/DirectX.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Geometory.h"
#include "../System/Sprite.h"
#include "../System/Input.h"
#include "SceneGame.h"
#include "../System/Defines.h"


#include <stdio.h>
#include <crtdbg.h>

// timeGetTime����̎g�p
#pragma comment(lib, "winmm.lib")

//--- �O���[�o���ϐ�
Scene* g_pScene;

//--- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// �G���g���|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//--- �ϐ��錾
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG message;

	// �E�B���h�N���X���̐ݒ�
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.hInstance = hInstance;
	wcex.lpszClassName = "Class Name";
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_CLASSDC | CS_DBLCLKS;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = wcex.hIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// �E�B���h�E�N���X���̓o�^
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, "Failed to RegisterClassEx", "Error", MB_OK);
		return 0;
	}

	// �E�B���h�E�̍쐬
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	DWORD style = WS_CAPTION | WS_SYSMENU;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&rect, style, false, exStyle);
	hWnd = CreateWindowEx(
		exStyle, wcex.lpszClassName,
		APP_TITLE, style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		HWND_DESKTOP,
		NULL, hInstance, NULL
	);

	// �E�B���h�E�̕\��
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// ����������
	if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)))
	{
		Uninit();
		UnregisterClass(wcex.lpszClassName, hInstance);
		return 0;
	}

	//--- FPS����
	timeBeginPeriod(1);
	DWORD countStartTime = timeGetTime();
	DWORD preExecTime = countStartTime;

	//--- �E�B���h�E�̊Ǘ�
	while (1)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&message, NULL, 0, 0))
			{
				break;
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		else
		{
			DWORD nowTime = timeGetTime();
			float diff = static_cast<float>(nowTime - preExecTime);
			if (diff >= 1000.0f / fFPS)
			{
				Update();
				Draw();
				preExecTime = nowTime;
			}
		}
	}


	// �I����
	timeEndPeriod(1);
	Uninit();
	UnregisterClass(wcex.lpszClassName, hInstance);

	return 0;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	// DirectX������
	hr = InitDirectX(hWnd, width, height, false);
	if (FAILED(hr)) { return hr; }

	// ���@�\������
	Geometory::Init();
	Sprite::Init();
	InitInput();
	ShaderList::Init();

	// �V�[��
	g_pScene = new SceneGame();

	return hr;
}

void Uninit()
{
	if (g_pScene) delete g_pScene;

	ShaderList::Uninit();
	UninitInput();
	Sprite::Uninit();
	Geometory::Uninit();
	UninitDirectX();
}

void Update()
{
	UpdateInput();
	g_pScene->RootUpdate();
}

void Draw()
{
	BeginDrawDirectX();

	// �����̕\��
//#ifdef _DEBUG
//	// �O���b�h
//	DirectX::XMFLOAT4 lineColor(0.5f, 0.5f, 0.5f, 1.0f);
//	float size = DEBUG_GRID_NUM * DEBUG_GRID_MARGIN;
//	for (int i = 1; i <= DEBUG_GRID_NUM; ++i)
//	{
//		float grid = i * DEBUG_GRID_MARGIN;
//		DirectX::XMFLOAT3 pos[2] = {
//			DirectX::XMFLOAT3(grid, 0.0f, size),
//			DirectX::XMFLOAT3(grid, 0.0f,-size),
//		};
//		Geometory::AddLine(pos[0], pos[1], lineColor);
//		pos[0].x = pos[1].x = -grid;
//		Geometory::AddLine(pos[0], pos[1], lineColor);
//		pos[0].x = size;
//		pos[1].x = -size;
//		pos[0].z = pos[1].z = grid;
//		Geometory::AddLine(pos[0], pos[1], lineColor);
//		pos[0].z = pos[1].z = -grid;
//		Geometory::AddLine(pos[0], pos[1], lineColor);
//	}
//	// ��
//	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(size,0,0), DirectX::XMFLOAT4(1,0,0,1));
//	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,size,0), DirectX::XMFLOAT4(0,1,0,1));
//	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,size), DirectX::XMFLOAT4(0,0,1,1));
//	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(-size,0,0),  DirectX::XMFLOAT4(0,0,0,1));
//	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,-size),  DirectX::XMFLOAT4(0,0,0,1));
//
//	Geometory::DrawLines();
//
//	// �J�����̒l
//	static bool camAutoSwitch = false;
//	static bool camUpDownSwitch = true;
//	static float camAutoRotate = 1.0f;
//	if (IsKeyTrigger(VK_RETURN)) {
//		camAutoSwitch ^= true;
//	}
//	if (IsKeyTrigger(VK_SPACE)) {
//		camUpDownSwitch ^= true;
//	}
//
//	DirectX::XMVECTOR camPos;
//	if (camAutoSwitch) {
//		camAutoRotate += 0.01f;
//	}
//	camPos = DirectX::XMVectorSet(
//		cosf(camAutoRotate) * 5.0f,
//		3.5f * (camUpDownSwitch ? 1.0f : -1.0f),
//		sinf(camAutoRotate) * 5.0f,
//		0.0f);
//
//	// �W�I���g���p�J����������
//	DirectX::XMFLOAT4X4 mat[2];
//	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
//		DirectX::XMMatrixLookAtLH(
//			camPos,
//			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
//			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
//		)));
//	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(
//		DirectX::XMMatrixPerspectiveFovLH(
//			DirectX::XMConvertToRadians(60.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f)
//	));
//	Geometory::SetView(mat[0]);
//	Geometory::SetProjection(mat[1]);
//#endif

	g_pScene->RootDraw();
	EndDrawDirectX();
}

// EOF