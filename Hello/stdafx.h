// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Leak detector
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h> 
#include <fstream> 
#include <crtdbg.h> 
#include <thread>

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dx9mesh.h>

#include <vector>
#include <string>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;

extern HWND m_hwnd;

class HelloMain;
extern HelloMain* g_HelloMain;

#define GetMainDevice() g_HelloMain->GetDevice()

#define WINSIZEX 640
#define WINSIZEY 360

#define NUM_SAMPLE 5000
#define MAX_DEPTH 5
#define DEBUG_RAYCOLOR _DEBUG

#define NUM_WORKERTHREAD 7

struct Vertex
{
	XMFLOAT3 vPosition;
	XMFLOAT3 vNormal;
};

struct VtxTex
{
	XMFLOAT3 vPosition;
	XMFLOAT2 vUV;
};

struct Index
{
	UINT _1;
	UINT _2;
	UINT _3;
};

struct TransConstantBuffer 
{
	XMFLOAT4X4 matWorld;
	XMFLOAT4X4 matView;
	XMFLOAT4X4 matProj;
};