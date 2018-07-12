#pragma once

class Ray
{
public:
	Ray(const XMFLOAT2& vPixelPos, const XMFLOAT3& vOrigin);
	~Ray();

	VOID SetDirToWorld();
	VOID IntersectObject();

	XMFLOAT2 m_vPixelPos;
	XMFLOAT3 m_vOrigin;
	XMFLOAT3 m_vDirection;
};