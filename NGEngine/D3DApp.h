#pragma once
#include "D3DAppBase.h"
class D3DApp:public D3DAppBase
{
public:
	D3DApp(UINT widht, UINT height, std::wstring name);
	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;
	void OnMouseDown(WPARAM btnState,int x,int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;
	void OnKeyDown(UINT8) override;
	void OnKeyUp(UINT8) override;
	void OnKeyboardInput()override;
};

