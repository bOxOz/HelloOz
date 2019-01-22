#pragma once

class Device
{
public:
	Device();
	~Device();

	VOID OnInit();
	VOID OnUpdate();
	VOID OnRender();
	VOID OnDestroy();

	ComPtr<ID3D12Device> GetDevice() { return m_pDevice; }

private:
	VOID PopulateCommandList();
	VOID WaitForPreviousFrame();

	VOID LoadPipeline();
	VOID LoadAssets();
	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	static const UINT nFrameCount = 2;

	ComPtr<ID3D12Device>				m_pDevice;
	ComPtr<ID3D12RootSignature>			m_pRootSignature;
	ComPtr<IDXGISwapChain3>				m_pSwapChain;
	ComPtr<ID3D12DescriptorHeap>		m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap>		m_pSRVHeap;
	ComPtr<ID3D12PipelineState>			m_pPipelineState;
	ComPtr<ID3D12CommandAllocator>		m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_pCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pCommandList;
	ComPtr<ID3D12Fence>					m_pFence;

	ComPtr<ID3D12Resource>				m_pRenderTargets[nFrameCount];
	ComPtr<ID3D12Resource>				m_pTexture;

	UINT m_nFrameIndex;
	UINT m_nRTVDescriptorSize;

	HANDLE m_pFenceEvent;
	UINT64 m_nFenceValue;
};