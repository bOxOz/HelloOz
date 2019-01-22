#include "stdafx.h"
#include "Sphere.h"
#include "Ray.h"


Sphere::Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT3& vEmittance, const XMFLOAT3& vColor, const FLOAT fSpecular/*=0.f*/)
{
	m_vPosition = vPosition;
	m_vScale = XMFLOAT3(fRadius, fRadius, fRadius);
	
	m_tMaterial.vEmittance = vEmittance;
	m_tMaterial.vBaseColor = vColor;
	m_tMaterial.fSpecular = fSpecular;

	if (m_tMaterial.vEmittance.x == 0.f && m_tMaterial.vEmittance.y == 0.f && m_tMaterial.vEmittance.z == 0.f)
		m_tMaterial.bEmitter = FALSE;
	else
		m_tMaterial.bEmitter = TRUE;
}

Sphere::~Sphere()
{
}

BOOL Sphere::Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm)
{
	XMVECTOR l = XMLoadFloat3(&XMFLOAT3(m_vPosition.x - ray.m_vOrigin.x, m_vPosition.y - ray.m_vOrigin.y, m_vPosition.z - ray.m_vOrigin.z));

	FLOAT s, l2;
	XMStoreFloat(&s, XMVector3Dot(l, XMLoadFloat3(&ray.m_vDirection)));
	XMStoreFloat(&l2, XMVector3Dot(l, l));

	FLOAT r2 = powf(m_vScale.x, 2);

	if (s < 0 && ((l2 > r2 || fabsf(l2 - r2) < 0.0001f) || r2 == 1.f))
		return FALSE;

	FLOAT m2 = l2 - powf(s, 2);

	if (m2 > r2)
		return FALSE;

	FLOAT q = sqrtf(r2 - m2);
	
	if (l2 > r2)
		fIntersectDist = s - q;
	else
		fIntersectDist = s + q;

	vIntersectPos = XMFLOAT3(ray.m_vOrigin.x + fIntersectDist * ray.m_vDirection.x,
							 ray.m_vOrigin.x + fIntersectDist * ray.m_vDirection.y,
							 ray.m_vOrigin.x + fIntersectDist * ray.m_vDirection.z);

	vIntersectNorm = XMFLOAT3(vIntersectPos.x - m_vPosition.x, vIntersectPos.y - m_vPosition.y, vIntersectPos.z - m_vPosition.z);
	
	return TRUE;
}

BOOL Sphere::IntersectP(const Ray& ray, FLOAT& fIntersectDist)
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

	return TRUE;
}