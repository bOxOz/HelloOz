#pragma once

struct Vertex
{
	DirectX::XMFLOAT3 vPosition;
	DirectX::XMFLOAT4 vColor;
};

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

	UINT GetWidth()	{ return m_nWidth; }
	UINT GetHeight() { return m_nHeight; }

private:
	VOID PopulateCommandList();
	VOID WaitForPreviousFrame();

	// Init
	VOID LoadPipeline();
	VOID LoadAssets();
	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	VOID ThrowIfFailed(HRESULT hr);

	UINT	m_nWidth;
	UINT	m_nHeight;
	FLOAT	m_fAspectRatio;

	Microsoft::WRL::ComPtr<ID3D12Resource>	m_pVB;
	D3D12_VERTEX_BUFFER_VIEW				m_tVBView;

	static const UINT nFrameCount = 2;

	Microsoft::WRL::ComPtr<ID3D12Device>				m_pDevice;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_pRootSignature;
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