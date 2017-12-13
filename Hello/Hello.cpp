#include "stdafx.h"
#include "Hello.h"
#include "d3dx12.h"

#include "Rect.h"
#include "Camera.h"
#include "Box.h"
#include "Light.h"
#include "Ray.h"
//#include "Sphere.h"

HelloMain::HelloMain()
	: m_pDevice(nullptr), m_pRootSignature(nullptr), m_pSwapChain(nullptr), m_pPipelineState(nullptr)
	, m_pCommandAllocator(nullptr), m_pCommandQueue(nullptr), m_pCommandList(nullptr), m_pFence(nullptr)
	, m_pRTVHeap(nullptr), m_pSRVHeap(nullptr), m_pTexture(nullptr), m_nFrameIndex(-1), m_nRTVDescriptorSize(0)
	, m_pFenceEvent(nullptr), m_nFenceValue(0)
	, m_pCamera(nullptr), m_pBox(nullptr), m_pRoom(nullptr), m_pLight(nullptr), m_pRenderRect(nullptr)
{
	ZeroMemory(m_pRayList, sizeof(Ray*) * WINSIZEX * WINSIZEY);
}

HelloMain::~HelloMain()
{

}

VOID HelloMain::OnInit()
{
	FLOAT fBoxSize = 1.f;
	FLOAT fRoomSize = 10.f;

	XMFLOAT3 vCameraPos{ 10.f, 10.f, -10.f };
	XMFLOAT3 vLightPos{ 4.f, 6.f, -5.f };

	// Create Object
	m_pCamera = new Camera(0.1f, 1000.f, vCameraPos, XMFLOAT3(0.f, 0.f, 0.0f));
	m_pLight = new Light(vLightPos);

	m_PixelColorList.reserve(WINSIZEX * WINSIZEY);

	for (INT y = 0; y < WINSIZEY; ++y)
	{
		for (INT x = 0; x < WINSIZEX; ++x)
		{
			m_pRayList[(y * WINSIZEX) + x] = new Ray(XMFLOAT2(FLOAT(x), FLOAT(y)));
			m_PixelColorList.push_back(XMFLOAT4(0.f, 0.f, 0.f, 1.f));
		}
	}

	m_pBox = new Box();
	m_pRoom = new Box();

	m_pBox->CreateShape(fBoxSize, XMFLOAT4(1.f, 0.9f, 0.1f, 1.f));
	m_pRoom->CreateShape(fRoomSize, XMFLOAT4(0.95f, 0.95f, 0.9f, 1.f), FALSE);

	// Check Intersect
	for (INT i = 0; i < WINSIZEX * WINSIZEY; ++i)
		m_pRayList[i]->IntersectObject();

	// Init Device
	LoadPipeline();
	LoadAssets();

	m_pRenderRect = new Rect();
	m_pRenderRect->CreateShape();

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

	delete m_pRenderRect;
	delete m_pCamera;
	delete m_pLight;
	delete m_pBox;
	delete m_pRoom;

	for (INT i = 0; i < WINSIZEX * WINSIZEY; ++i)
		delete m_pRayList[i];
	
	m_PixelColorList.empty();

	OutputDebugString(L"Destroy Device\n");
}

VOID HelloMain::PopulateCommandList()
{
	ThrowIfFailed(m_pCommandAllocator->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), m_pPipelineState.Get()));

	// Set necessary state.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_pSRVHeap.Get() };
	m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_pCommandList->SetGraphicsRootDescriptorTable(0, m_pSRVHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12_VIEWPORT vp { 0.f, 0.f, static_cast<FLOAT>(WINSIZEX), static_cast<FLOAT>(WINSIZEY), 0.f, 1.f };
	m_pCommandList->RSSetViewports(1, &vp);

	D3D12_RECT rc { 0, 0, static_cast<LONG>(WINSIZEX), static_cast<LONG>(WINSIZEY) };
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
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Record commands
	const FLOAT clearColor[] { 1.f, 0.68f, 0.788f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_pCommandList->IASetVertexBuffers(0, 1, &m_pRenderRect->m_tVBView);
	m_pCommandList->DrawInstanced(m_pRenderRect->m_nVBSize / sizeof(VtxTex), 1, 0, 0);

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
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{ WINSIZEX, WINSIZEY, DXGI_FORMAT_R8G8B8A8_UNORM, FALSE, { 1, 0 },
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
		// RTV
		// D3D12_DESCRIPTOR_HEAP_DESC { Type, NumDescriptors, Flags, NodeMask }
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, nFrameCount, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pRTVHeap)));

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV , 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE , 0 };
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSRVHeap)));

		m_nRTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	
	// Frame Resources
	{
		// RTV
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
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData {};

		// D3D12_DESCRIPTOR_RANGE1 { RangeType, NumDescriptors, BaseShaderRegister, RegisterSpace, Flags, OffsetInDescriptorsFromTableStart }
		D3D12_DESCRIPTOR_RANGE1 ranges[1] { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND };

		// D3D12_ROOT_PARAMETER1 { ParameterType, (DescriptorTable, Constants, Descriptor), ShaderVisibility }
		D3D12_ROOT_PARAMETER1 rootParameters[1] { D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, 1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL };

		D3D12_STATIC_SAMPLER_DESC sampler {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// D3D12_VERSIONED_ROOT_SIGNATURE_DESC { Version, (Desc_1_0, Desc_1_1 { NumParameters, *pParameters, NumStaticSamplers, *pStaticSamplers, Flags  }) }
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{
			D3D_ROOT_SIGNATURE_VERSION_1_1, _countof(rootParameters), (D3D12_ROOT_PARAMETER*)rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &pSignature, &pError));
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
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };

		// D3D12_DEPTH_STENCIL_DESC { DepthEnable, DepthWriteMask, DepthFunc, StencilEnable, 
		//							  StencilReadMask, StencilWriteMask, 
		//							  FrontFace, BackFace }
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc { FALSE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS, FALSE,
													D3D12_DEFAULT_STENCIL_READ_MASK , D3D12_DEFAULT_STENCIL_WRITE_MASK,
													defaultStencilOp, defaultStencilOp };


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
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ inputElementDescs, _countof(inputElementDescs) };
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));
	}

	// Create Command List
	ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pCommandList)));

	ComPtr<ID3D12Resource> textureUploadHeap;

	// Create the texture.
	{
		// D3D12_RESOURCE_DESC { Dimension, Alignment, Width, Height, DepthOrArraySize, MipLevels, Format, SampleDesc, Layout, Flags }
		D3D12_RESOURCE_DESC textureDesc { D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, WINSIZEX, WINSIZEY, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
										  { 1, 0 }, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE };

		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pTexture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_pTexture.Get(), 0, 1);

		// Create the GPU upload buffer.
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));

		// D3D12_SUBRESOURCE_DATA { pData, RowPitch, SlicePitch } 
		D3D12_SUBRESOURCE_DATA textureData { &m_PixelColorList[0], WINSIZEX * sizeof(XMFLOAT4), textureData.RowPitch * WINSIZEY };

		UpdateSubresources(m_pCommandList.Get(), m_pTexture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		// D3D12_SHADER_RESOURCE_VIEW_DESC { Format, ViewDimension, Shader4ComponentMapping, Texture2D }
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		m_pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, m_pSRVHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Command List
	ThrowIfFailed(m_pCommandList->Close());
	ID3D12CommandList* ppCommandLists[]{ m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

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

VOID ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}