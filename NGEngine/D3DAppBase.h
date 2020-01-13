#pragma once
class D3DAppBase
{
public:
	D3DAppBase(UINT width, UINT height, std::wstring name);
	virtual ~D3DAppBase();
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnKeyDown(UINT8) {}
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {};
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {};
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {};
	virtual void OnKeyUp(UINT8) {}
	virtual void OnKeyboardInput() {};

	int GetWidht()const { return mWidth; }
	int GetHeight()const { return mHeight; }
	const WCHAR* GetTitle() { return mTitle.c_str(); }
protected:
	int mWidth;
	int mHeight;
private:
	std::wstring mTitle;
};

