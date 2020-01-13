#pragma once
#include "D3DAppBase.h"
class Win32Application
{
public:
	static int Run(D3DAppBase* appBase,HINSTANCE hInstance, int mCmdShow);
	static HWND GetHwnd() { return mHwnd; }
protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	static HWND mHwnd;
};

