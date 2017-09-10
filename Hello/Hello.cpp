#include "stdafx.h"
#include "Hello.h"

using namespace Microsoft::WRL;
using namespace DirectX;

HelloMain::HelloMain(UINT nWidth, UINT nHeight)
	: m_nWidth(nWidth), m_nHeight(nHeight)
	, m_pDevice(nullptr), m_pSwapChain(nullptr), m_pRTVHeap(nullptr), m_pPipelineState(nullptr)
	, m_pCommandAllocator(nullptr), m_pCommandQueue(nullptr), m_pCommandList(nullptr), m_pFence(nullptr)
	, m_nFrameIndex(-1), m_nRTVDescriptorSize(0)
	, m_pFenceEvent(nullptr), m_nFenceValue(0)
{
	memset(m_pRenderTargets, 0, sizeof(m_pRenderTargets));
}

HelloMain::~HelloMain()
{

}

VOID HelloMain::OnInit()
{
	OutputDebugString(L"OnInit()\n");

	// Factory
	UINT dxgiFactoryFlags = 0;

	ComPtr<IDXGIFactory4> pFactory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));

	// Adapter
	ComPtr<IDXGIAdapter1> pAdapter;
	GetHardwareAdapter(pFactory.Get(), &pAdapter);

	// Device
	ThrowIfFailed(D3D12CreateDevice(
		pAdapter.Get(),
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_pDevice)
	));

	// CommandQueue
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
	}

	// Swap chain
	ComPtr<IDXGISwapChain1> pSwapChain;
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = nFrameCount;
		swapChainDesc.Width = m_nWidth;
		swapChainDesc.Height = m_nHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ThrowIfFailed(pFactory->CreateSwapChainForHwnd(
			m_pCommandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
			m_hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&pSwapChain
		));
	}

	// This sample does not support fullscreen transitions. 라고 한다.
	ThrowIfFailed(pFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(pSwapChain.As(&m_pSwapChain));
	m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// Descriptor Heaps
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = nFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap)));

		m_nRTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Frame Resources
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < nFrameCount; n++)
		{
			ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n])));
			m_pDevice->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += m_nRTVDescriptorSize;
		}
	}

	// Command Allocator
	ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));

	// Command List
	ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));
	ThrowIfFailed(m_pCommandList->Close());

	// Create Fence
	{
		ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
		m_nFenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_pFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_pFenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}

VOID HelloMain::OnUpdate()
{
	OutputDebugString(L"OnUpdate()\n");
}

VOID HelloMain::OnRender()
{
	OutputDebugString(L"OnRender()\n");

	ThrowIfFailed(m_pCommandAllocator->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), m_pPipelineState.Get()));

	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pRenderTargets[m_nFrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr += m_nFrameIndex * m_nRTVDescriptorSize;

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pCommandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_pCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_pSwapChain->Present(1, 0));

	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_nFenceValue;
	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), fence));
	m_nFenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_pFence->SetEventOnCompletion(fence, m_pFenceEvent));
		WaitForSingleObject(m_pFenceEvent, INFINITE);
	}

	m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

VOID HelloMain::OnDestroy()
{
	OutputDebugString(L"OnDestroy()\n");

	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_nFenceValue;
	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), fence));
	m_nFenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_pFence->SetEventOnCompletion(fence, m_pFenceEvent));
		WaitForSingleObject(m_pFenceEvent, INFINITE);
	}

	m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

VOID HelloMain::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			break;
	}

	*ppAdapter = adapter.Detach();
}

VOID HelloMain::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}
