#pragma once

class Ray
{
public:
	Ray(const XMFLOAT2& vPixelPos);
	~Ray();

	void SetDir();
	void IntersectObject();

	XMFLOAT2 m_vPixelPos;
	XMFLOAT3 m_vDir;
};