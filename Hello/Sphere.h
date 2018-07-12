#pragma once

#include "Primitive.h"

class Ray;
class Sphere : public Primitive
{
public:
	Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT4& vColor);
	virtual ~Sphere();

	BOOL Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm);
};

