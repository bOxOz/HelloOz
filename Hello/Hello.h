#pragma once

class Primitive;
class Rect;
class Camera;
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
	void SetPixelColor(const XMFLOAT2& vPixelPos, const XMFLOAT4& vColor) 
	{ 
		UINT idx = UINT(vPixelPos.x) + (UINT(vPixelPos.y) * WINSIZEX);
		m_PixelColorList[idx] = vColor;
	}

	std::vector<Primitive*> m_ObjectList;

	Light*			m_pLight;
	Rect*			m_pRenderRect;

private:
	VOID PopulateCommandList();
	VOID WaitForPreviousFrame();

	// Init
	VOID LoadPipeline();
	VOID LoadAssets();
	VOID GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	Camera*					m_pCamera;
	Ray*					m_pRayList[WINSIZEX * WINSIZEY];
	std::vector<XMFLOAT4>	m_PixelColorList;

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

VOID ThrowIfFailed(HRESULT hr);