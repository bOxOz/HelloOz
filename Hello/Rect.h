#pragma once

#include "Primitive.h"

class Ray;
class Rect : public Primitive
{
public:
	Rect();
	virtual ~Rect();

	VOID CreateShape();

	BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm) { return TRUE; }
	BOOL IntersectP(const Ray& ray, FLOAT& fIntersectDist) { return TRUE; }

//private:
	ComPtr<ID3D12Resource>		m_pVB;
	D3D12_VERTEX_BUFFER_VIEW	m_tVBView;
	UINT						m_nVBSize;
};