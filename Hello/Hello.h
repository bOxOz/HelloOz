#pragma once

class Camera;
class Box;
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
	ComPtr<ID3D12Device> GetDevice() { return m_pDevice; }

private:
	VOID PopulateCommandList();
	VOID WaitForPreviousFrame();

	// Init
	VOID LoadPipeline();
	VOID LoadAssets();
	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	UINT	m_nWidth;
	UINT	m_nHeight;
	FLOAT	m_fAspectRatio;

	Box*	m_pBox;
	Camera*	m_pCamera;

	static const UINT nFrameCount = 2;

	ComPtr<ID3D12Device>				m_pDevice;
	ComPtr<ID3D12RootSignature>			m_pRootSignature;
	ComPtr<IDXGISwapChain3>				m_pSwapChain;
	ComPtr<ID3D12DescriptorHeap>		m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap>		m_pCBVHeap;
	ComPtr<ID3D12PipelineState>			m_pPipelineState;
	ComPtr<ID3D12CommandAllocator>		m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_pCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pCommandList;
	ComPtr<ID3D12Fence>					m_pFence;

	ComPtr<ID3D12Resource>				m_pRenderTargets[nFrameCount];
	ComPtr<ID3D12Resource>				m_pTransConstantBuffer;
	UINT8*								m_pCBVDataBegin;

	UINT m_nFrameIndex;
	UINT m_nRTVDescriptorSize;
	
	HANDLE m_pFenceEvent;
	UINT64 m_nFenceValue;
};

VOID ThrowIfFailed(HRESULT hr);