#pragma once

class Rect
{
public:
	Rect();
	~Rect();

	void CreateShape();

	ComPtr<ID3D12Resource>		m_pVB;
	D3D12_VERTEX_BUFFER_VIEW	m_tVBView;
	UINT						m_nVBSize;
};