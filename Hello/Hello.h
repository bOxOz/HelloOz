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

	ComPtr<ID3D12Device> GetDevice() { return m_pD3D12Device->GetDevice(); }
	Camera* GetCamera() { return m_pCamera; }

	std::vector<Primitive*> m_ObjectList;
	Camera*					m_pCamera;
	Ray*					m_pRayList[WINSIZEX * WINSIZEY];

	std::vector<XMFLOAT4>	m_PixelColorList;
	Rect*					m_pRenderRect;


private:
	Device*					m_pD3D12Device;
};

VOID ThrowIfFailed(HRESULT hr);
bool SaveImage(const std::string& szPathName, const std::vector<char>& lpBits, int w, int h);