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
}

HelloMain::~HelloMain()
{

}

VOID HelloMain::OnInit()
{
	CreateObject();

#if NUM_WORKERTHREAD
	std::thread* workerThread[NUM_WORKERTHREAD] { nullptr };
	for (int iWorkerThread = 0; iWorkerThread < NUM_WORKERTHREAD; ++iWorkerThread)
	{
		INT nQuota = (WINSIZEX * WINSIZEY) / (NUM_WORKERTHREAD + 1);
		INT iStartPixel = nQuota * (iWorkerThread + 1);

		std::thread* worker = new std::thread(DoPathTracing, iWorkerThread + 1, &m_PixelColorList[0]);
		workerThread[iWorkerThread] = worker;
	}
#endif
	
	DoPathTracing(0, &m_PixelColorList[0]);
	
#if NUM_WORKERTHREAD
	for (int iWorkerThread = 0; iWorkerThread < NUM_WORKERTHREAD; ++iWorkerThread)
	{
		std::thread* worker = workerThread[iWorkerThread];
		worker->join();
		delete worker;
	
		workerThread[iWorkerThread] = nullptr;
	}
#endif

	SaveImage("../bin/test.bmp");

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

	for (INT i = 0; i < m_ObjectList.size(); ++i)
		delete m_ObjectList[i];

	m_ObjectList.empty();
	m_PixelColorList.empty();
}

VOID HelloMain::CreateObject()
{
	// Camera
	XMFLOAT3 vCameraPos(0.f, -8.5f, -7.f);
	m_pCamera = new Camera(0.1f, 1000.f, vCameraPos, XMFLOAT3(0.f, -8.5f, 0.0f));

	for (INT y = 0; y < WINSIZEY; ++y)
	{
		for (INT x = 0; x < WINSIZEX; ++x)
		{
			m_PixelColorList.push_back(XMFLOAT4(0.f, 0.f, 0.f, 1.f));
		}
	}

	// Sphere
	//m_ObjectList.push_back(new Sphere(XMFLOAT3(0.f, -3.f, 0.f), 3.f, XMFLOAT3(1.f, 1.f, 1.f), XMFLOAT3(1.f, 1.f, 1.f)));
	
	m_ObjectList.push_back(new Sphere(XMFLOAT3(0.f, -8.5f, 3.5f), 1.5f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 0.f, 0.f), BxDF_REFLECTION));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(4.f, -7.7f, 3.6f), 2.3f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(1.f, 0.f, 0.f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(2.f, -9.f, 0.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(1.f, 1.f, 0.f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(-1.f, -9.5f, 0.f), 0.5f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(-2.5f, -9.f, -1.f), 1.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 1.f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(-3.8f, -8.f, 3.5f), 2.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 0.f, 1.f)));
	m_ObjectList.push_back(new Sphere(XMFLOAT3(-2.f, -9.3f, 8.f), 0.7f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(1.f, 0.f, 1.f)));

	// Room
	m_ObjectList.push_back(new Box(XMFLOAT3(0.f, 0.f, 0.f), 20.f, XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(1.f, 1.f, 1.f), FALSE));
}

VOID DoPathTracing(INT iThread, XMFLOAT4* PixelColorList)
{
	INT nQuota = (WINSIZEX * WINSIZEY) / (NUM_WORKERTHREAD + 1);
	INT iStartPixel = nQuota * iThread;
	INT iEndPixel = nQuota * (iThread + 1);

	XMFLOAT3 vCameraPos = g_HelloMain->GetCamera()->GetPosition();

	for (int iPixel = iStartPixel; iPixel < iEndPixel; ++iPixel)
	{
		INT nHit = 0;
		XMFLOAT4 vAddResult(0.f, 0.f, 0.f, 1.f);

		for (INT iSample = 0; iSample < NUM_SAMPLE; ++iSample)
		{
			INT nDepth = 1;
			Ray newRay = Ray(XMFLOAT2(FLOAT(iPixel % WINSIZEX), FLOAT(iPixel / WINSIZEX)), vCameraPos);

			XMFLOAT4 res = Ray::TracePath(&newRay, nDepth);
			if (res.x > 0.3f || res.y > 0.3f || res.z > 0.3f)
			{
				++nHit;
				vAddResult = XMFLOAT4(vAddResult.x + res.x, vAddResult.y + res.y, vAddResult.z + res.z, 1.f);
			}
		}

		if (nHit)
			PixelColorList[iPixel] = XMFLOAT4(vAddResult.x / nHit, vAddResult.y / nHit, vAddResult.z / nHit, 1.f);
	}
}

VOID ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}

bool SaveImage(const std::string& szPathName) {
	std::ofstream pFile(szPathName, std::ios_base::binary);
	if (!pFile.is_open())
		return false;

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = WINSIZEX;
	bmih.biHeight = WINSIZEY;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = WINSIZEX * WINSIZEY * 3;

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
	std::streamoff nWrittenFileHeaderSize = pFile.tellp();

	// And then the bitmap info header
	pFile.write((const char*)&bmih, sizeof(BITMAPINFOHEADER));
	std::streamoff nWrittenInfoHeaderSize = pFile.tellp();

	// Finally, write the image data itself
	//-- the data represents our drawing
	std::vector<char> pixel;
	pixel.reserve(WINSIZEX * WINSIZEY);

	std::vector<XMFLOAT4> colorList = g_HelloMain->GetColorList();

	for (INT y = WINSIZEY - 1; y >= 0; --y)
	{
		for (INT x = 0; x < WINSIZEX; ++x)
		{
			INT i = (y * WINSIZEX) + x;
			pixel.push_back(char(colorList[i].z * 255));
			pixel.push_back(char(colorList[i].y * 255));
			pixel.push_back(char(colorList[i].x * 255));
		}
	}
	
	pFile.write(&pixel[0], pixel.size());
	std::streamoff nWrittenDIBDataSize = pFile.tellp();
	pFile.close();

	return true;
}