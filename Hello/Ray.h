#pragma once

class Primitive;
class Ray
{
public:
	Ray(const XMFLOAT2& vPixelPos, const XMFLOAT3& vOrigin);
	Ray(const XMFLOAT3& vStart, const XMFLOAT3& vEnd);
	~Ray();

	VOID SetDirToWorld();
	BOOL IntersectObject();

	static XMFLOAT4 TracePath(Ray* pRay, INT& Depth);
	static XMFLOAT3 RandomUnitVectorInHemisphereOf(const XMFLOAT3& vNormal);

	XMFLOAT2 m_vPixelPos;
	XMFLOAT3 m_vOrigin;
	XMFLOAT3 m_vDirection;

	Primitive*	m_pHitObj;
	XMFLOAT3	m_HitPosition;
	XMFLOAT3	m_HitNormal;
};