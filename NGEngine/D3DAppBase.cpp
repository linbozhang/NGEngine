#include "stdafx.h"
#include "D3DAppBase.h"
#include "D3DUtil.h"
#include "Win32Application.h"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

/**
 * \brief
 * \param width
 * \param height
 * \param title
 */
D3DAppBase::D3DAppBase(const UINT width, const UINT height, const wstring title) : mWidth(width), mHeight(height),
mTitle(title)
{
	
}
D3DAppBase::~D3DAppBase()
{

}

void D3DAppBase::OnInit()
{
	// Init Direct3D
#if defined(DEBUG)||defined(_DEBUG)
	// enable the d3d12 debug layer
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(mDxgiFactory.GetAddressOf())));

	// Try to create hardware device
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr, 
		D3D_FEATURE_LEVEL_11_0, 
		IID_PPV_ARGS(mDevice.GetAddressOf()));
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(mDevice.GetAddressOf())));
	}

	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf())));
	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 在DirectX12中，MSAA不能像DX11中直接设置使用

#ifdef _DEBUG
	// LogAdapters
	LogAdapters();
#endif

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mCommandQueue.GetAddressOf())));
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetAddressOf())));
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));
	// 我们首次引用 command list的时候会调用reset方法，而reset方法调用前要求 command list需要 close
	mCommandList->Close();

	// 释放之前的swapchain 我们会重新创建
	mSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;// m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	sd.OutputWindow = Win32Application::GetHwnd();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// swap chain uses queue to perform flush
	ThrowIfFailed(mDxgiFactory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf()));


	// CreateRtvAndDsvDescriptorHeaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
	OnResize();
}

void D3DAppBase::OnResize()
{
	assert(mDevice);
	assert(mSwapChain);
	assert(mCommandAllocator);
	FlushCommandQueue();
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));
	// release the previous resources we will be recreating
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();

	// resize the swap chain
	ThrowIfFailed(mSwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, mWidth, mHeight, mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	mCurBackBufferIndex = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mSwapChainBuffer[i].GetAddressOf())));
		mDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE, 
		&depthStencilDesc, 
		D3D12_RESOURCE_STATE_COMMON, 
		&optClear, 
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());
	// Transition the resource from its initial state to be used as a depth buffer.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	// Execute the resize commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mWidth);
	mScreenViewport.Height = static_cast<float>(mHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mWidth, mHeight };
}

void D3DAppBase::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mFenceValue++;
	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mFenceValue));
	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mFenceValue)
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, nullptr);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(mFence->SetEventOnCompletion(mFenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* D3DAppBase::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurBackBufferIndex].Get();
}
D3D12_CPU_DESCRIPTOR_HANDLE D3DAppBase::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 
		mCurBackBufferIndex, 
		mRtvDescriptorSize
	);
}



D3D12_CPU_DESCRIPTOR_HANDLE D3DAppBase::DepthStencilView() const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void D3DAppBase::LogAdapters()
{
	{
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		vector<IDXGIAdapter*> adapterList;
		while (mDxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);
			wstring text = L"***Adapter";
			text += desc.Description;
			text += L"\n";
			OutputDebugString(text.c_str());
			adapterList.push_back(adapter);
			++i;
		}
		for (size_t i = 0; i < adapterList.size(); ++i)
		{
			LogAdapterOutputs(adapterList[i]);
			ReleaseCom(adapterList[i]);
		}
	}
}

void D3DAppBase::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);
		wstring text = L"***Output";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());
		LogOutputDisplayModes(output, mBackBufferFormat);
		ReleaseCom(output);
		++i;
	}
}
void D3DAppBase::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;
	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);
	vector<DXGI_MODE_DESC> modeList(count);
	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		wstring text =
			L"Width=" + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";
		OutputDebugString(text.c_str());
	}

}