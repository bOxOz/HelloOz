#include "stdafx.h"
#include "Rect.h"
#include "Hello.h"

Rect::Rect()
	: m_pVB(nullptr), m_nVBSize(0)
{
	CreateShape();
}

Rect::~Rect()
{
}

VOID Rect::CreateShape()
{
	// Create the vertex buffer.
	{
		VtxTex arrVertices[]
		{
			{ { -1.f, 1.f, 0.f },	{ 0.f, 0.f } },
			{ { 1.f, 1.f, 0.f },	{ 1.f, 0.f } },
			{ { 1.f, -1.f, 0.f },	{ 1.f, 1.f } },
			{ { -1.f, 1.f, 0.f },	{ 0.f, 0.f } },
			{ { 1.f, -1.f, 0.f },	{ 1.f, 1.f } },
			{ { -1.f, -1.f, 0.f },	{ 0.f, 1.f } }
		};

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
		m_tVBView = { m_pVB->GetGPUVirtualAddress(), m_nVBSize, sizeof(VtxTex) };
	}
}