#pragma once

#include "Device.h"

class Primitive;
class Rect;
class Camera;
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

	VOID CreateObject();

	ComPtr<ID3D12Device> GetDevice() { return m_pD3D12Device->GetDevice(); }
	Camera* GetCamera() { return m_pCamera; }
	std::vector<XMFLOAT4>& GetColorList() { return m_PixelColorList; }

	std::vector<Primitive*> m_ObjectList;
	Camera*					m_pCamera;

	std::vector<XMFLOAT4>	m_PixelColorList;
	Rect*					m_pRenderRect;

#if DEBUG_RAYCOLOR
	std::vector<XMFLOAT4>	m_DebugPixelColorList[WINSIZEX * WINSIZEY];
#endif

private:
	Device*					m_pD3D12Device;
};

VOID DoPathTracing(INT iThread, XMFLOAT4* PixelColorList);
VOID ThrowIfFailed(HRESULT hr);
bool SaveImage(const std::string& szPathName);