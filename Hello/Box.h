#pragma once

class Box
{
public:
	Box();
	~Box();

	void SetScale(FLOAT fScale);

	void CreateShape(const XMFLOAT4& vColor, BOOL bBackCull = TRUE);
	void SetTransConstantBuffer(UINT8* pCBVDataBegin, const XMFLOAT4X4& matProj, const XMFLOAT4X4& matView);

	Vertex						m_arrVertices[8];
	UINT						m_arrIndeces[36];

	ComPtr<ID3D12Resource>		m_pVB;
	D3D12_VERTEX_BUFFER_VIEW	m_tVBView;
	ComPtr<ID3D12Resource>		m_pIB;
	D3D12_INDEX_BUFFER_VIEW		m_tIBView;

	UINT m_nVBSize;
	UINT m_nIBSize;
	UINT m_nIdxCnt;

	XMFLOAT4X4 m_matWorld;
};