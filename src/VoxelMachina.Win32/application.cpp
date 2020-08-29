#include "pch.h"
#include "assert.h"
#include "application.h"
#include "coreGraphics.h"
#include "input.h"
#include "systemTime.h"
#include <iostream>
#include <string>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

LONG defaultWidth = 600;
LONG defaultHeight = 400;

int64_t previewsFrameTick = 0;

HWND g_coreWindow;

void InitializeApplication(IGameApp& app)
{
	graphics::Initialize();
	SystemTime::Initialize();
	Input::Initialize();
	app.Startup();
}

bool UpdateApplication(IGameApp& app)
{
	auto currentTick = SystemTime::GetCurrentTick();
	auto deltaTime = SystemTime::TimeBetweenTicks(previewsFrameTick, currentTick);
	previewsFrameTick = currentTick;

	Input::Update(deltaTime);
	app.Update(deltaTime);
	app.RenderScene();

	double fps = 1.0 / deltaTime;
	std::string titlebar = "FPS: " + std::to_string(fps);

	SetWindowTextA(g_coreWindow, titlebar.c_str());

	Input::PostUpdate();

	return !app.IsDone();
}

void TerminateApplication(IGameApp& app)
{
}

//Creates the window and manages the lifetime of the application.
void RunApplication(IGameApp& app, HINSTANCE instance, const wchar_t* className)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = LoadIcon(instance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(instance, IDI_APPLICATION);

	assert(0 != RegisterClassEx(&wcex), "Unable to register a window");

	// Create window
	RECT rc = { 0, 0, defaultWidth, defaultHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_coreWindow = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);

	assert(g_coreWindow != 0);

	InitializeApplication(app);

	ShowWindow(g_coreWindow, SW_SHOWDEFAULT);

	do
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;
	} while (UpdateApplication(app));    // Returns false to quit loop

	TerminateApplication(app);
}

// Called every time the application receives a message
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		graphics::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
		break;

	case WM_KEYDOWN:
		Input::SetKey(wParam, true);
		break;

	case WM_CHAR:
		Input::SetKey(wParam, false);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}