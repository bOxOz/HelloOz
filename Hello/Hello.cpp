#include "stdafx.h"
#include "Hello.h"

using namespace Microsoft::WRL;
using namespace DirectX;

HelloMain::HelloMain(UINT nWidth, UINT nHeight)
	: m_nWidth(nWidth), m_nHeight(nHeight)
	, m_pDevice(nullptr), m_pRootSignature(nullptr), m_pSwapChain(nullptr), m_pRTVHeap(nullptr), m_pPipelineState(nullptr)
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

	ID3D12CommandList* ppCommandLists[] { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

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

	// Set necessary state.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	D3D12_VIEWPORT vp { 0.f, 0.f, static_cast<FLOAT>(m_nWidth), static_cast<FLOAT>(m_nHeight) };
	m_pCommandList->RSSetViewports(1, &vp);

	D3D12_RECT rc { 0, 0, static_cast<LONG>(m_nWidth), static_cast<LONG>(m_nHeight) };
	m_pCommandList->RSSetScissorRects(1, &rc);

	// Indicate that the back buffer will be used as a render target.
	// D3D12_RESOURCE_TRANSITION_BARRIER { pResource, Subresource, StateBefore, StateAfter }
	D3D12_RESOURCE_TRANSITION_BARRIER transBarrier { m_pRenderTargets[m_nFrameIndex].Get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET };

	// D3D12_RESOURCE_BARRIER { Type, Flags, union(Transition, Aliasing, UAV) }
	D3D12_RESOURCE_BARRIER barrier { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, transBarrier };

	m_pCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr += m_nFrameIndex * m_nRTVDescriptorSize;

	m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands
	const FLOAT clearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_tVBView);
	m_pCommandList->DrawInstanced(3, 1, 0, 0);

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
	++m_nFenceValue;

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

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> pFactory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));

	// Adapter
	ComPtr<IDXGIAdapter1> pAdapter;
	GetHardwareAdapter(pFactory.Get(), &pAdapter);

	// Device
	ThrowIfFailed(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)));

	// Command Queue
	{
		// D3D12_COMMAND_QUEUE_DESC { Type, Priority, Flags, NodeMask }
		D3D12_COMMAND_QUEUE_DESC queueDesc;
		ZeroMemory(&queueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));

		ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
	}

	// Swap Chain
	ComPtr<IDXGISwapChain1> pSwapChain;
	{
		// DXGI_SWAP_CHAIN_DESC1 { Width, Height, Format, Stereo, SampleDesc, 
		//						   BufferUsage, BufferCount, Scaling, SwapEffect, AlphaMode, Flags }
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{ m_nWidth, m_nHeight, DXGI_FORMAT_R8G8B8A8_UNORM, FALSE, { 1, 0 }, 
			DXGI_USAGE_RENDER_TARGET_OUTPUT, nFrameCount, DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, 0 };

		ThrowIfFailed(pFactory->CreateSwapChainForHwnd(
			m_pCommandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
			m_hwnd,
			&swapChainDesc,
			nullptr, // pFullscreenDesc
			nullptr, // pRestrictToOutput
			&pSwapChain
		));
	}

	// This sample does not support full screen transitions.
	ThrowIfFailed(pFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(pSwapChain.As(&m_pSwapChain));
	m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// Descriptor Heaps
	{
		// D3D12_DESCRIPTOR_HEAP_DESC { Type, NumDescriptors, Flags, NodeMask }
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, nFrameCount, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };

		// Create a render target view (RTV) descriptor heap.
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
		// D3D12_ROOT_SIGNATURE_DESC { NumParameters, pParameters, NumStaticSamplers, pStaticSamplers, Flags }
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{ 0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };
		
		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError));
		ThrowIfFailed(m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;

#endif
		ComPtr<ID3DBlob> pVertexShader;
		ComPtr<ID3DBlob> pPixelShader;

		ThrowIfFailed(D3DCompileFromFile(L"../bin/Shader/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"../bin/Shader/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, nullptr));

		// Input Layout.
		// D3D12_INPUT_ELEMENT_DESC { SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate }
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// D3D12_BLEND_DESC { AlphaToCoverageEnable, IndependentBlendEnable, RenderTarget }
		D3D12_BLEND_DESC blendDesc{ 0 };
		{
			// D3D12_RENDER_TARGET_BLEND_DESC { BlendEnable, LogicOpEnable, SrcBlend, DestBlend, 
			//									BlendOp, SrcBlendAlpha, DestBlendAlpha, LogicOp, RenderTargetWriteMask }
			D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc
			{
				FALSE,FALSE,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};

			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
		}

		// D3D12_RASTERIZER_DESC { FillMode, CullMode, FrontCounterClockwise, DepthBias, DepthBiasClamp, 
		//						   SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable, ForcedSampleCount, ConservativeRaster }
		D3D12_RASTERIZER_DESC rasterizerDesc { D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE ,FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF };

		// Pipeline State Object
		// D3D12_GRAPHICS_PIPELINE_STATE_DESC { pRootSignature, VS, PS, DS, HS, GS, StreamOutput, BlendState, SampleMask, 
		//										RasterizerState, DepthStencilState, InputLayout, IBStripCutValue, PrimitiveTopologyType,
		//										NumRenderTargets, RTVFormats, DSVFormat, SampleDesc, NodeMask, CachedPSO, Flags }
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{ 0 };
		psoDesc.pRootSignature = m_pRootSignature.Get();
		psoDesc.VS = D3D12_SHADER_BYTECODE{ pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize() };
		psoDesc.PS = D3D12_SHADER_BYTECODE{ pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize() };
		psoDesc.BlendState = blendDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ inputElementDescs, _countof(inputElementDescs) };
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));
	}

	// Create the vertex buffer.
	{
		// Vertex { vPosition, vColor }
		Vertex arrVertices[]
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

		// D3D12_VERTEX_BUFFER_VIEW { BufferLocation, SizeInBytes, StrideInBytes } 
		m_tVBView = { m_pVB->GetGPUVirtualAddress(), nVBSize, sizeof(Vertex) };
	}

	// Command List
	ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pCommandList)));
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
