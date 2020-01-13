#include "stdafx.h"
#include "Win32Application.h"
#include <windowsx.h>
HWND Win32Application::mHwnd = nullptr;
int Win32Application::Run(D3DAppBase* app, HINSTANCE hInstance, int nCmdShow) {
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	LocalFree(argv);
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"NGEngine";
	RegisterClassEx(&windowClass);
	RECT windowRect = { 0,0,static_cast<LONG>(app->GetWidht()),static_cast<LONG>(app->GetHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	mHwnd = CreateWindow(
		windowClass.lpszClassName,
		app->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		app,
		);
	app->OnInit();
	ShowWindow(mHwnd, nCmdShow);
	UpdateWindow(mHwnd);
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			app->OnUpdate();
			app->OnRender();
		}
	}
	app->OnDestroy();
	return static_cast<char>(msg.wParam);
}

LRESULT Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	D3DAppBase* app = reinterpret_cast<D3DAppBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (message) {
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;
	case WM_MOUSEMOVE:
		if (app) {
			app->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (app) {
			app->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (app) {
			app->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return 0;
	case WM_KEYDOWN:
		if (app)
		{
			app->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_KEYUP:
		if (app)
		{
			app->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}