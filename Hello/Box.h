#pragma once

#include "Primitive.h"

class Box : public Primitive
{
public:
	Box(const XMFLOAT3& vPosition, FLOAT fScale, const XMFLOAT4& vColor, BOOL bBackCull);
	virtual ~Box();

	VOID CreateShape(BOOL bBackCull);

	BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm);

private:
	std::vector<Vertex>		m_arrVertex;
};