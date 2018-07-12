#pragma once

class Ray;
class Primitive
{
public:
	Primitive();
	virtual ~Primitive();

	const XMFLOAT3& GetPostion() { return m_vPosition; }
	const XMFLOAT3& GetScale() { return m_vScale; }
	const XMFLOAT4& GetColor() { return m_vColor; }

	virtual BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm) = 0;

protected:
	XMFLOAT3 m_vPosition;
	XMFLOAT3 m_vScale;
	XMFLOAT4 m_vColor;
};

