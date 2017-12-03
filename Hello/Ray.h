#pragma once

class Ray
{
public:
	Ray(const XMFLOAT2& vPixelPos);
	~Ray();

	void IntersectObject();

private:
	void SetNormal();

	XMFLOAT2 m_vPixelPos;
	XMFLOAT3 m_vNormal;
};