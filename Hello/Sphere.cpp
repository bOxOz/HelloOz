#include "stdafx.h"
#include "Sphere.h"
#include "Ray.h"


Sphere::Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT4& vColor)
{
	m_vPosition = vPosition;
	m_vScale = XMFLOAT3(fRadius, fRadius, fRadius);
	m_vColor = vColor;
}

Sphere::~Sphere()
{
}

BOOL Sphere::Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm)
{
	XMVECTOR l = XMLoadFloat3(&m_vPosition) - XMLoadFloat3(&ray.m_vOrigin);

	FLOAT s, l2;
	XMStoreFloat(&s, XMVector3Dot(l, XMLoadFloat3(&ray.m_vDirection)));
	XMStoreFloat(&l2, XMVector3Dot(l, l));

	FLOAT r2 = powf(m_vScale.x, 2);

	if (s < 0 && l2 > r2)
		return FALSE;

	FLOAT m2 = l2 - powf(s, 2);

	if (m2 > r2)
		return FALSE;

	FLOAT q = sqrtf(r2 - m2);
	
	if (l2 > r2)
		fIntersectDist = s - q;
	else
		fIntersectDist = s + q;

	XMStoreFloat3(&vIntersectPos, XMLoadFloat3(&ray.m_vOrigin) + fIntersectDist * XMLoadFloat3(&ray.m_vDirection));
	XMStoreFloat3(&vIntersectNorm, XMLoadFloat3(&vIntersectPos) - XMLoadFloat3(&m_vPosition));
	
	return TRUE;
}
