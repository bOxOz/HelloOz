#include "stdafx.h"
#include "Ray.h"

#include "Hello.h"
#include "Camera.h"
#include "Primitive.h"

Ray::Ray(const XMFLOAT2& vPixelPos, const XMFLOAT3& vOrigin)
	: m_vPixelPos(vPixelPos), m_vOrigin(vOrigin), m_vDir(XMFLOAT3(0.f, 0.f, 0.f)), m_pHitObj(nullptr)
{
	SetDirToWorld();
}

Ray::Ray(const XMFLOAT3& vOrigin, const XMFLOAT3& vDirection)
	: m_vOrigin(vOrigin), m_vDir(vDirection)
{
}

Ray::~Ray()
{
}

VOID Ray::SetDirToWorld()
{
	const XMFLOAT4X4& matProj = g_HelloMain->GetCamera()->GetProj();
	m_vDir.x = ((m_vPixelPos.x / (WINSIZEX >> 1)) - 1.f) / matProj._11;
	m_vDir.y = ((-m_vPixelPos.y / (WINSIZEY >> 1)) + 1.f) / matProj._22;
	m_vDir.z = 1.f;

	const XMFLOAT4X4& matView = g_HelloMain->GetCamera()->GetView();
	XMMATRIX matViewInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&matView));
	XMVECTOR vRayNormal = XMVector3Normalize(XMLoadFloat3(&m_vDir));
	vRayNormal = XMVector3TransformNormal(vRayNormal, matViewInv);

	XMStoreFloat3(&m_vDir, vRayNormal);
}

BOOL Ray::IntersectObject()
{
	XMFLOAT3 vIntersectPos { 0.f, 0.f, 0.f };
	XMFLOAT3 vIntersectNorm { 0.f, 0.f, 0.f };

	FLOAT fDist = 1000.f;
	FLOAT fMinDist = 1000.f;

	for (auto obj : g_HelloMain->m_ObjectList)
	{
		if (obj->Intersect(*this, fDist, vIntersectPos, vIntersectNorm))
		{
			if (fDist < fMinDist)
			{
				fMinDist = fDist;

				m_pHitObj = obj;
				m_HitPosition = vIntersectPos;

				XMStoreFloat3(&m_HitNormal, XMVector3Normalize(XMLoadFloat3(&vIntersectNorm)));
			}
		}
	}

	return (m_pHitObj != nullptr) ? TRUE : FALSE;
}

#define BLACK XMFLOAT4(0.f, 0.f, 0.f, 1.f)
#define WHITE XMFLOAT4(1.f, 1.f, 1.f, 1.f)

XMFLOAT4 Ray::TracePath(Ray* pRay, INT& Depth)
{
	if (Depth > MAX_DEPTH)
		return BLACK;
	
	if (pRay->IntersectObject() == FALSE)
		return WHITE;

	Material material = pRay->m_pHitObj->GetMaterial();

	XMFLOAT3 newDir;
	if (material.fSpecular > 0.f)
	{
		XMVECTOR rayDir = XMLoadFloat3(&pRay->m_vDir);
		XMVECTOR objNomral = XMLoadFloat3(&pRay->m_HitNormal);
		XMFLOAT3 reflectDir;
		XMStoreFloat3(&reflectDir, rayDir + (2 * objNomral * XMVector3Dot(-rayDir, objNomral)));
	}
	else
	{
		newDir = RandomUnitVectorInHemisphereOf(pRay->m_HitNormal);
	}
	
	Ray newRay(pRay->m_HitPosition, newDir);

	XMFLOAT4 reflected = XMFLOAT4(0.f, 0.f, 0.f, 0.f);

	if(material.bEmitter == FALSE)
		reflected = TracePath(&newRay, ++Depth);

	FLOAT cos_theta = 1.f;
	XMStoreFloat(&cos_theta, XMVector3Dot(XMLoadFloat3(&newRay.m_vDir), XMLoadFloat3(&pRay->m_HitNormal)));

	XMFLOAT3 BRDF = XMFLOAT3(material.vBaseColor.x * cos_theta, material.vBaseColor.y * cos_theta, material.vBaseColor.z * cos_theta);
	
	XMFLOAT4 res = XMFLOAT4(min(material.vEmittance.x + ((BRDF.x * reflected.x) * (1.f - material.fSpecular)) + (reflected.x * material.fSpecular), 1.f),
							min(material.vEmittance.y + ((BRDF.y * reflected.y) * (1.f - material.fSpecular)) + (reflected.y * material.fSpecular), 1.f),
							min(material.vEmittance.z + ((BRDF.z * reflected.z) * (1.f - material.fSpecular)) + (reflected.z * material.fSpecular), 1.f), 1.f);

	return res;
}

FLOAT rand_float()
{
	// return -1.f ~ 1.f
	return ((rand() % 20000) * 0.0001f) - 1.f;
}

XMFLOAT3 Ray::RandomUnitVectorInHemisphereOf(const XMFLOAT3& vNormal)
{
	XMVECTOR randomVector = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(rand_float(), rand_float(), rand_float())));
	randomVector = randomVector + XMLoadFloat3(&vNormal);
	randomVector = XMVector3Normalize(randomVector);

	XMFLOAT3 result;
	XMStoreFloat3(&result, randomVector);

	return result;
}
