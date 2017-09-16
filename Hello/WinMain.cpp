// Hello.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WinMain.h"
#include "Hello.h"

HWND m_hwnd = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HelloMain hello(1280, 720);

    // Perform application initialization:
	WNDCLASSEX wcex { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.lpszClassName = L"Hello";
	RegisterClassExW(&wcex);

	RECT rcWindow = { 0, 0, static_cast<LONG>(hello.GetWidth()), static_cast<LONG>(hello.GetHeight()) };
	AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		wcex.lpszClassName,
		L"Hello",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rcWindow.right - rcWindow.left,
		rcWindow.bottom - rcWindow.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		nullptr);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	hello.OnInit();

	ShowWindow(m_hwnd, nCmdShow);

    // Main message loop:
	MSG msg {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		hello.OnUpdate();
		hello.OnRender();
	}

	hello.OnDestroy();

	_CrtDumpMemoryLeaks();
    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}