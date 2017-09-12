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
	, m_pVB(nullptr)
{
	ZeroMemory(m_pRenderTargets, 0);
	ZeroMemory(&m_tVBView, 0);
	m_fAspectRatio = static_cast<FLOAT>(m_nWidth) / static_cast<FLOAT>(m_nHeight);
}

HelloMain::~HelloMain()
{

}

VOID HelloMain::OnInit()
{
	LoadPipeline();
	LoadAssets();

	OutputDebugString(L"Init Device\n");
}

VOID HelloMain::OnUpdate()
{
}

VOID HelloMain::OnRender()
{
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_pSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

VOID HelloMain::OnDestroy()
{
	WaitForPreviousFrame();
	CloseHandle(m_pFenceEvent);

	OutputDebugString(L"Destroy Device\n");
}

VOID HelloMain::PopulateCommandList()
{
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
}

VOID HelloMain::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 nFence = m_nFenceValue;
	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), nFence));
	m_nFenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < nFence)
	{
		ThrowIfFailed(m_pFence->SetEventOnCompletion(nFence, m_pFenceEvent));
		WaitForSingleObject(m_pFenceEvent, INFINITE);
	}

	m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

// Init Device 
VOID HelloMain::LoadPipeline()
{
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
}

VOID HelloMain::LoadAssets()
{
	// Create an empty root signature.
	{
		//CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		//
		//ComPtr<ID3DBlob> signature;
		//ComPtr<ID3DBlob> error;
		//ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		//ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		//ComPtr<ID3DBlob> vertexShader;
		//ComPtr<ID3DBlob> pixelShader;
		//
		//ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr));
		//ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr));
		//
		//// Define the vertex input layout.
		//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//};
		//
		//// Describe and create the graphics pipeline state object (PSO).
		//D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		//psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		//psoDesc.pRootSignature = m_rootSignature.Get();
		//psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		//psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		//psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState.DepthEnable = FALSE;
		//psoDesc.DepthStencilState.StencilEnable = FALSE;
		//psoDesc.SampleMask = UINT_MAX;
		//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//psoDesc.NumRenderTargets = 1;
		//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		//psoDesc.SampleDesc.Count = 1;
		//ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// Create the vertex buffer.
	{
		// Vertex { vPosition, vColor }
		Vertex arrVertices[] =
		{
			{ { 0.0f, 0.25f * m_fAspectRatio, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_fAspectRatio, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_fAspectRatio, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT nVBSize = sizeof(arrVertices);

		// D3D12_HEAP_PROPERTIES { Type, CPUPageProperty, MemoryPoolPreference, CreationNodeMask, VisibleNodeMask }
		D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

		// D3D12_RESOURCE_DESC { Dimension, Alignment, Width, Height, DepthOrArraySize, MipLevels, 
		//						 Format, SampleDesc.Count, SampleDesc.Quality, Layout, Flags }
		D3D12_RESOURCE_DESC resourceDesc{ D3D12_RESOURCE_DIMENSION_BUFFER, 0, nVBSize, 1, 1, 1,
			DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pVB)));

		UINT8* pVertexDataBegin;
		D3D12_RANGE readRange{ 0, 0 };

		ThrowIfFailed(m_pVB->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, arrVertices, nVBSize);
		m_pVB->Unmap(0, nullptr);

		// D3D12_VERTEX_BUFFER_VIEW { BufferLocation, StrideInBytes, SizeInBytes } 
		m_tVBView = { m_pVB->GetGPUVirtualAddress(), sizeof(Vertex) , nVBSize };
	}

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

		WaitForPreviousFrame();
	}
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

// End - Init Device

VOID HelloMain::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}
