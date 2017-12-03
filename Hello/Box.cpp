#include "stdafx.h"
#include "Box.h"
#include "Hello.h"

Box::Box()
	: m_pVB(nullptr), m_pIB(nullptr)
	, m_nVBSize(0), m_nIBSize(0), m_nIdxCnt(0)
{
	XMStoreFloat4x4(&m_matWorld, XMMatrixIdentity());
}

Box::~Box()
{
}

void Box::SetScale(FLOAT fScale)
{
	XMStoreFloat4x4(&m_matWorld, XMMatrixScaling(fScale, fScale, fScale));

	for (UINT i = 0; i < 8; ++i)
	{
		m_arrVertices[i].vPosition.x *= fScale;
		m_arrVertices[i].vPosition.y *= fScale;
		m_arrVertices[i].vPosition.z *= fScale;
	}
}

void Box::SetTransConstantBuffer(UINT8* pCBVDataBegin, const XMFLOAT4X4& matProj, const XMFLOAT4X4& matView)
{
	TransConstantBuffer cb;
	XMStoreFloat4x4(&cb.matWorld, XMMatrixTranspose(XMLoadFloat4x4(&m_matWorld)));
	XMStoreFloat4x4(&cb.matView, XMMatrixTranspose(XMLoadFloat4x4(&matView)));
	XMStoreFloat4x4(&cb.matProj, XMMatrixTranspose(XMLoadFloat4x4(&matProj)));

	memcpy(pCBVDataBegin, &cb, sizeof(cb));
}

void Box::CreateShape(const XMFLOAT4& vColor, BOOL bBackCull)
{
	// Create the vertex buffer.
	{
		Vertex arrVertices[]
		{
			{ { -0.5f, 0.5f, -0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { 0.5f, 0.5f, -0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { 0.5f, -0.5f, -0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { -0.5f, -0.5f, -0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { -0.5f, 0.5f, 0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { 0.5f, 0.5f, 0.5f },		vColor, { 0.f, 0.f, 0.f } },
			{ { 0.5f, -0.5f, 0.5f },	vColor, { 0.f, 0.f, 0.f } },
			{ { -0.5f, -0.5f, 0.5f },	vColor, { 0.f, 0.f, 0.f } }
		};

		// Normal °è»ê
		if (bBackCull)
		{
			for (INT idx = 0; idx < sizeof(arrVertices) / sizeof(Vertex); ++idx)
				XMStoreFloat3(&arrVertices[idx].vNormal, XMVector3Normalize(XMLoadFloat3(&arrVertices[idx].vPosition)));
		}
		else
		{
			for (INT idx = 0; idx < sizeof(arrVertices) / sizeof(Vertex); ++idx)
				XMStoreFloat3(&arrVertices[idx].vNormal, XMVector3Normalize(XMLoadFloat3(&arrVertices[idx].vPosition) * -1.f));
		}

		m_nVBSize = sizeof(arrVertices);

		// D3D12_HEAP_PROPERTIES { Type, CPUPageProperty, MemoryPoolPreference, CreationNodeMask, VisibleNodeMask }
		D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

		// D3D12_RESOURCE_DESC { Dimension, Alignment, Width, Height, DepthOrArraySize, MipLevels, 
		//						 Format, SampleDesc.Count, SampleDesc.Quality, Layout, Flags }
		D3D12_RESOURCE_DESC resourceDesc{ D3D12_RESOURCE_DIMENSION_BUFFER, 0, m_nVBSize, 1, 1, 1,
			DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

		ThrowIfFailed(GetMainDevice()->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pVB)));

		UINT8* pVertexDataBegin;
		D3D12_RANGE readRange{ 0, 0 };
		ThrowIfFailed(m_pVB->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, arrVertices, m_nVBSize);
		m_pVB->Unmap(0, nullptr);

		// D3D12_VERTEX_BUFFER_VIEW { BufferLocation, SizeInBytes, StrideInBytes } 
		m_tVBView = { m_pVB->GetGPUVirtualAddress(), m_nVBSize, sizeof(Vertex) };

		memcpy(m_arrVertices, arrVertices, m_nVBSize);
	}

	{
		UINT arrIndex[]{
			1, 5, 6,
			1, 6, 2,
			4, 0, 3,
			4, 3, 7,
			4, 5, 1,
			4, 1, 0,
			3, 2, 6,
			3, 6, 7,
			7, 6, 5,
			7, 5, 4,
			0, 1, 2,
			0, 2, 3
		};
		
		if(!bBackCull)
		{
			UINT arrFrontCullIndex[]{
				6, 5, 1,
				2, 6, 1,
				3, 0, 4,
				7, 3, 4,
				1, 5, 4,
				0, 1, 4,
				6, 2, 3,
				7, 6, 3,
				5, 6, 7,
				4, 5, 7,
				2, 1, 0,
				3, 2, 0
			};

			memcpy(arrIndex, arrFrontCullIndex, sizeof(arrIndex));
		};
		m_nIBSize = sizeof(arrIndex);
		m_nIdxCnt = sizeof(arrIndex) / sizeof(UINT);

		// D3D12_HEAP_PROPERTIES { Type, CPUPageProperty, MemoryPoolPreference, CreationNodeMask, VisibleNodeMask }
		D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

		// D3D12_RESOURCE_DESC { Dimension, Alignment, Width, Height, DepthOrArraySize, MipLevels, 
		//						 Format, SampleDesc.Count, SampleDesc.Quality, Layout, Flags }
		D3D12_RESOURCE_DESC resourceDesc{ D3D12_RESOURCE_DIMENSION_BUFFER, 0, m_nIBSize, 1, 1, 1,
			DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

		ThrowIfFailed(GetMainDevice()->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pIB)));

		UINT8* pIndexDataBegin;
		D3D12_RANGE readRange{ 0, 0 };
		ThrowIfFailed(m_pIB->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, arrIndex, m_nIBSize);
		m_pIB->Unmap(0, nullptr);

		// D3D12_VERTEX_BUFFER_VIEW { BufferLocation, SizeInBytes, StrideInBytes } 
		m_tIBView = { m_pIB->GetGPUVirtualAddress(), m_nIBSize, DXGI_FORMAT_R32_UINT };

		memcpy(m_arrIndeces, arrIndex, m_nIBSize);
	}
}