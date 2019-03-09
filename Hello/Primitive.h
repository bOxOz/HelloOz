#pragma once

enum BxDFTYPE {
	BxDF_DIFFUSE = 1 << 0,
	BxDF_REFLECTION = 1 << 1,
	BxDF_TRANSMISSION = 1 << 2,
};

struct Material {
	XMFLOAT3 vEmittance;
	XMFLOAT3 vBaseColor;
	BxDFTYPE eType;

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

