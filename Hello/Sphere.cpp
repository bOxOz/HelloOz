#include "stdafx.h"
#include "Sphere.h"
#include "Ray.h"


Sphere::Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT3& vEmittance, const XMFLOAT3& vColor, const FLOAT fSpecular/*=0.f*/)
{
	m_vPosition = vPosition;
	m_fRadius = fRadius;
	
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
	XMVECTOR po = XMLoadFloat3(&XMFLOAT3(m_vPosition.x - ray.m_vOrigin.x, m_vPosition.y - ray.m_vOrigin.y, m_vPosition.z - ray.m_vOrigin.z));

	FLOAT b, c;
	XMStoreFloat(&b, XMVector3Dot(XMLoadFloat3(&ray.m_vDir), po));
	XMStoreFloat(&c, XMVector3Dot(po, po));
	c = c - m_fRadius * m_fRadius;

	FLOAT discrim = b * b - c;
	if (discrim < SMALL_NUMBER)
		return FALSE;

	FLOAT rootDiscrim = sqrtf(discrim);

	FLOAT t0 = b + rootDiscrim;
	FLOAT t1 = b - rootDiscrim;

	if (t0 < SMALL_NUMBER && t1 < SMALL_NUMBER)
		return FALSE;

	fIntersectDist = (t0 < t1) ? t0 : t1;

	vIntersectPos = XMFLOAT3(ray.m_vOrigin.x + fIntersectDist * ray.m_vDir.x,
							 ray.m_vOrigin.y + fIntersectDist * ray.m_vDir.y,
							 ray.m_vOrigin.z + fIntersectDist * ray.m_vDir.z);

	vIntersectNorm = XMFLOAT3(vIntersectPos.x - m_vPosition.x, vIntersectPos.y - m_vPosition.y, vIntersectPos.z - m_vPosition.z);

	return TRUE;
}

BOOL Sphere::IntersectP(const Ray& ray, FLOAT& fIntersectDist)
{
	return TRUE;
}