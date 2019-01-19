#pragma once

struct Material {
	XMFLOAT3 emittance;
	XMFLOAT3 reflectance;

	BOOL bEmitter;
};

class Ray;
class Primitive
{
public:
	Primitive();
	virtual ~Primitive();

	const XMFLOAT3& GetPostion() { return m_vPosition; }
	const XMFLOAT3& GetScale() { return m_vScale; }
	const Material& GetMaterial() { return m_tMaterial; }

	virtual BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm) = 0;
	virtual BOOL IntersectP(const Ray& ray, FLOAT& fIntersectDist) = 0;

protected:
	XMFLOAT3 m_vPosition;
	XMFLOAT3 m_vScale;
	Material m_tMaterial;
};

