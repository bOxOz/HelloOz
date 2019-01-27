#pragma once

#include "Primitive.h"

class Ray;
class Sphere : public Primitive
{
public:
	Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT3& vEmittance, const XMFLOAT3& vColor, const FLOAT fSpecular = 0.f);
	virtual ~Sphere();

	BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm);
	BOOL IntersectP(const Ray& ray, FLOAT& fIntersectDist);

protected:
	FLOAT m_fRadius;
};

