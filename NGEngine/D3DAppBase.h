#pragma once
class D3DAppBase
{
public:
	D3DAppBase(UINT width, UINT height, std::wstring title);
	virtual ~D3DAppBase();
	virtual void OnInit() ;
	virtual void OnResize();
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
	float mAspectRation;
	Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>mCommandList;
	static const UINT SWAP_CHAIN_BUFFER_COUNT = 2;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT mCurBackBufferIndex;
	HANDLE mFenceEvent;	
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mFenceValue;
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map < std::string, Microsoft::WRL::ComPtr<ID3DBlob >> mShaders;
	Microsoft::WRL::ComPtr<ID3DBlob>mVsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob>mPsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
	
	void FlushCommandQueue();
	
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	
private:
	std::wstring mTitle;
};

