#include "stdafx.h"
#include "Hello.h"

#include "Camera.h"
#include "Ray.h"
#include "Rect.h"
#include "Box.h"
#include "Sphere.h"

HelloMain::HelloMain()
	: m_pD3D12Device(nullptr), m_pCamera(nullptr)
{
	ZeroMemory(m_pRayList, sizeof(Ray*) * WINSIZEX * WINSIZEY);
}

HelloMain::~HelloMain()
{

}

VOID HelloMain::OnInit()
{
	CreateObject();

	// Check Intersect
	int nSampleNum = 5000;
	for (int i = 0; i < WINSIZEX * WINSIZEY; ++i)
	{
		INT nHit = 0;
	
		for (INT sample = 0; sample < nSampleNum; ++sample)
		{
			XMFLOAT4 res = Ray::TracePath(m_pRayList[i]);
			bool bHit = (res.x > 0.f || res.y > 0.f || res.z > 0.f);
	
			if (bHit)
			{
				++nHit;
				m_PixelColorList[i] = XMFLOAT4(m_PixelColorList[i].x + res.x, m_PixelColorList[i].y + res.y, m_PixelColorList[i].z + res.z, 1.f);
				//m_DebugPixelColorList[i].push_back(res);
			}
		}
	
		if(nHit)
			m_PixelColorList[i] = XMFLOAT4(m_PixelColorList[i].x / nHit, m_PixelColorList[i].y / nHit, m_PixelColorList[i].z / nHit, 1.f);
	}

	std::vector<char> pixel;
	pixel.reserve(WINSIZEX * WINSIZEY);

	for (INT y = WINSIZEY - 1; y >= 0; --y)
	{
		for (INT x = 0; x < WINSIZEX; ++x)
		{
			INT i = (y * WINSIZEX) + x;
			pixel.push_back(m_PixelColorList[i].z * 255);
			pixel.push_back(m_PixelColorList[i].y * 255);
			pixel.push_back(m_PixelColorList[i].x * 255);
		}
	}

	SaveImage("../bin/test.bmp", pixel, WINSIZEX, WINSIZEY);

	// Device
	m_pD3D12Device = new Device();
	m_pD3D12Device->OnInit();

	// Render Rect
	m_pRenderRect = new Rect();
	m_pRenderRect->CreateShape();
}

VOID HelloMain::OnUpdate()
{
	m_pD3D12Device->OnUpdate();
}

VOID HelloMain::OnRender()
{
	m_pD3D12Device->OnRender();
}

VOID HelloMain::OnDestroy()
{
	m_pD3D12Device->OnDestroy();

	delete m_pRenderRect;
	delete m_pCamera;

	for (INT i = 0; i < WINSIZEX * WINSIZEY; ++i)
		delete m_pRayList[i];

	for (INT i = 0; i < m_ObjectList.size(); ++i)
		delete m_ObjectList[i];

	m_ObjectList.empty();
	m_PixelColorList.empty();
}

VOID HelloMain::CreateObject()
{
	// Camera
	XMFLOAT3 vCameraPos(0.f, 0.f, -20.f);
	m_pCamera = new Camera(0.1f, 1000.f, vCameraPos, XMFLOAT3(0.f, -7.f, 0.0f));

	for (INT y = 0; y < WINSIZEY; ++y)
	{
		for (INT x = 0; x < WINSIZEX; ++x)
		{
			m_pRayList[(y * WINSIZEX) + x] = new Ray(XMFLOAT2(FLOAT(x), FLOAT(y)), vCameraPos);
			m_PixelColorList.push_back(XMFLOAT4(0.f, 0.f, 0.f, 1.f));
		}
	}

	// Sphere
	m_ObjectList.push_back(new Sphere(XMFLOAT3(0.f, -7.f, 2.f), 3.f, XMFLOAT3(1.f, 1.f, 1.f), XMFLOAT3(1.f, 1.f, 1.f)));

	m_ObjectList.push_back(new Sphere(XMFLOAT3(-4.f, -9.f, 2.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.9f, 0.3f, 0.3f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(-2.f, -9.f, -2.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.9f, 0.6f, 0.3f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(2.f, -9.f, -2.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.9f, 0.9f, 0.3f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(4.f, -9.f, 2.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.3f, 0.6f, 0.3f)));

	// Room
	m_ObjectList.push_back(new Box(XMFLOAT3(0.f, 0.f, 0.f), 20.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.95f, 0.95f, 0.9f), FALSE));
}

VOID ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}

bool SaveImage(const std::string& szPathName, const std::vector<char>& lpBits, int w, int h) {
	std::ofstream pFile(szPathName, std::ios_base::binary);
	if (!pFile.is_open())
		return false;

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = w * h * 3;

	BITMAPFILEHEADER bmfh;
	int nBitsOffset = sizeof(BITMAPFILEHEADER) + bmih.biSize;
	LONG lImageSize = bmih.biSizeImage;
	LONG lFileSize = nBitsOffset + lImageSize;
	bmfh.bfType = 'B' + ('M' << 8);
	bmfh.bfOffBits = nBitsOffset;
	bmfh.bfSize = lFileSize;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

	// Write the bitmap file header
	pFile.write((const char*)&bmfh, sizeof(BITMAPFILEHEADER));
	UINT nWrittenFileHeaderSize = pFile.tellp();

	// And then the bitmap info header
	pFile.write((const char*)&bmih, sizeof(BITMAPINFOHEADER));
	UINT nWrittenInfoHeaderSize = pFile.tellp();

	// Finally, write the image data itself
	//-- the data represents our drawing
	pFile.write(&lpBits[0], lpBits.size());
	UINT nWrittenDIBDataSize = pFile.tellp();
	pFile.close();

	return true;
}