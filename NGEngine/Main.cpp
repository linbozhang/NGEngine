#include "stdafx.h"
#include "Win32Application.h"
#include "D3DApp.h"
_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	D3DApp app(1280, 720, L"NGEngine");
	return Win32Application::Run(&app,hInstance, nCmdShow);
}