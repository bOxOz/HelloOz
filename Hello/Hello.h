#pragma once

class Camera;
class Box;
class Light;
class Ray;
class HelloMain
{
public:
	HelloMain();
	~HelloMain();

	VOID OnInit();
	VOID OnUpdate();
	VOID OnRender();
	VOID OnDestroy();

	VOID OnKeyDown(UINT8) {};
	VOID OnKeyUp(UINT8) {};

	ComPtr<ID3D12Device> GetDevice() { return m_pDevice; }
	Camera* GetCamera() { return m_pCamera; }
	void SetPixelColor(const XMFLOAT2& vPixelPos, const XMFLOAT3& vColor) 
	{ 
		UINT idx = UINT(vPixelPos.x) + (UINT(vPixelPos.y) * WINSIZEX);
		*(m_pPixelColor[idx]) = vColor; 
	}

	Box*			m_pBox;
	Box*			m_pRoom;
	Light*			m_pLight;

private:
	VOID PopulateCommandList1();
	VOID PopulateCommandList2();
	VOID WaitForPreviousFrame();

	// Init
	VOID LoadPipeline();
	VOID LoadAssets();
	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	Camera*			m_pCamera;
	Ray*			m_pRayList[WINSIZEX * WINSIZEY];
	XMFLOAT3*		m_pPixelColor[WINSIZEX * WINSIZEY];

	static const UINT nFrameCount = 2;

	ComPtr<ID3D12Device>				m_pDevice;
	ComPtr<ID3D12RootSignature>			m_pRootSignature;
	ComPtr<IDXGISwapChain3>				m_pSwapChain;
	ComPtr<ID3D12DescriptorHeap>		m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap>		m_pDSVHeap;
	ComPtr<ID3D12DescriptorHeap>		m_pCBVHeap;
	ComPtr<ID3D12PipelineState>			m_pPipelineState;
	ComPtr<ID3D12CommandAllocator>		m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_pCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pCommandList;
	ComPtr<ID3D12Fence>					m_pFence;

	ComPtr<ID3D12Resource>				m_pRenderTargets[nFrameCount];
	ComPtr<ID3D12Resource>				m_pResTexture;
	ComPtr<ID3D12Resource>				m_pDepthStencilBuffer;
	ComPtr<ID3D12Resource>				m_pTransConstantBuffer;
	UINT8*								m_pCBVDataBegin;

	UINT m_nFrameIndex;
	UINT m_nRTVDescriptorSize;
	
	HANDLE m_pFenceEvent;
	UINT64 m_nFenceValue;
};

VOID ThrowIfFailed(HRESULT hr);