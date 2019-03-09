#include "stdafx.h"
#include "Sphere.h"
#include "Ray.h"


Sphere::Sphere(const XMFLOAT3& vPosition, FLOAT fRadius, const XMFLOAT3& vEmittance, const XMFLOAT3& vColor, const BxDFTYPE eType)
{
	m_vPosition = vPosition;
	m_fRadius = fRadius;
	
	m_tMaterial.vEmittance = vEmittance;
	m_tMaterial.vBaseColor = vColor;
	m_tMaterial.eType = eType;

	if (m_tMaterial.vEmittance.x == 0.f && m_tMaterial.vEmittance.y == 0.f && m_tMaterial.vEmittance.z == 0.f)
		m_tMaterial.bEmitter = FALSE;
	else
		m_tMaterial.bEmitter = TRUE;
}

Sphere::~Sphere()
{
}

thread_local BOOL bInRay = FALSE;

BOOL Sphere::Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm)
{
	XMVECTOR oc = XMLoadFloat3(&XMFLOAT3(ray.m_vOrigin.x - m_vPosition.x, ray.m_vOrigin.y - m_vPosition.y, ray.m_vOrigin.z - m_vPosition.z));

	FLOAT b, c;
	XMStoreFloat(&b, XMVector3Dot(XMLoadFloat3(&ray.m_vDir), oc));
	b *= 2.f;

	XMStoreFloat(&c, XMVector3Dot(oc, oc));
	c = c - (m_fRadius * m_fRadius);

	FLOAT discrim = b * b - 4.f * c;
	if (discrim < 0.f)
		return FALSE;

	FLOAT rootDiscrim = sqrtf(discrim);

	FLOAT t0 = (-b + rootDiscrim) / 2.f;
	FLOAT t1 = (-b - rootDiscrim) / 2.f;

	if (t0 < 0.f && t1 < 0.f)
		return FALSE;

	fIntersectDist = (t0 < t1 || t1 < SMALL_NUMBER) ? t0 : t1;

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