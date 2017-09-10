#pragma once

class HelloMain
{
public:
	HelloMain(UINT nWidth, UINT nHeight);
	~HelloMain();

	VOID OnInit();
	VOID OnUpdate();
	VOID OnRender();
	VOID OnDestroy();

	VOID OnKeyDown(UINT8) {};
	VOID OnKeyUp(UINT8) {};

	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	VOID ThrowIfFailed(HRESULT hr);

	UINT m_nWidth;
	UINT m_nHeight;

	static const UINT nFrameCount = 2;

	Microsoft::WRL::ComPtr<ID3D12Device>				m_pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>				m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_pRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_pRenderTargets[nFrameCount];
	Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pPipelineState;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_pCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_pCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence>					m_pFence;

	UINT m_nFrameIndex;
	UINT m_nRTVDescriptorSize;
	
	HANDLE m_pFenceEvent;
	UINT64 m_nFenceValue;
};