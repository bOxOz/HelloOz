#include "stdafx.h"
#include "Ray.h"
#include "Hello.h"
#include "Camera.h"
#include "Box.h"
#include "Sphere.h"
#include "Light.h"

Ray::Ray(const XMFLOAT2& vPixelPos, const XMFLOAT3& vOrigin)
	: m_vPixelPos(vPixelPos), m_vOrigin(vOrigin), m_vDirection(XMFLOAT3(0.f, 0.f, 0.f))
{
	SetDirToWorld();
}

Ray::Ray(const XMFLOAT3& vStart, const XMFLOAT3& vEnd)
	: m_vPixelPos(XMFLOAT2(0.f, 0.f)), m_vOrigin(vStart)
{
	XMVECTOR vRayNormal = XMLoadFloat3(&vEnd) - XMLoadFloat3(&vStart);
	vRayNormal = XMVector3Normalize(vRayNormal);
	XMStoreFloat3(&m_vDirection, vRayNormal);
}

Ray::~Ray()
{

}

VOID Ray::SetDirToWorld()
{
	const XMFLOAT4X4& matProj = g_HelloMain->GetCamera()->GetProj();
	m_vDirection.x = ((m_vPixelPos.x / (WINSIZEX >> 1)) - 1.f) / matProj._11;
	m_vDirection.y = ((-m_vPixelPos.y / (WINSIZEY >> 1)) + 1.f) / matProj._22;
	m_vDirection.z = 1.f;

	const XMFLOAT4X4& matView = g_HelloMain->GetCamera()->GetView();
	XMMATRIX matViewInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&matView));
	XMVECTOR vRayNormal = XMVector3Normalize(XMLoadFloat3(&m_vDirection));
	vRayNormal = XMVector3TransformNormal(vRayNormal, matViewInv);

	XMStoreFloat3(&m_vDirection, vRayNormal);
}

VOID Ray::IntersectObject()
{
	XMFLOAT3 vResultPos { 0.f, 0.f, 0.f };
	XMFLOAT3 vResultNormal { 0.f, 0.f, 0.f };
	XMFLOAT4 vResultColor{ 0.f, 0.f, 0.f, 0.f };

	XMFLOAT3 vIntersectPos { 0.f, 0.f, 0.f };
	XMFLOAT3 vIntersectNorm { 0.f, 0.f, 0.f };

	FLOAT fDist = 1000.f, fMinDist = 1000.f;
	
	Primitive* colObj = NULL;

	for (auto obj : g_HelloMain->m_ObjectList)
	{
		if (obj->Intersect(*this, fDist, vIntersectPos, vIntersectNorm))
		{
			fMinDist = fDist;
			colObj = obj;

			vResultPos = vIntersectPos;
			vResultNormal = vIntersectNorm;
			vResultColor = obj->GetColor();
		}
	}

	// 교차하는 물체를 찾았다면 Light와 계산하여 Diffuse값을 결정
	if (fMinDist < 1000.f)
	{
		XMVECTOR vLightPos = XMLoadFloat3(&g_HelloMain->m_pLight->GetPosition()); 
		XMVECTOR vLightDir = (XMLoadFloat3(&vResultPos) - vLightPos) * -1.f;		
		vLightDir = XMVector3Normalize(vLightDir);

		XMVECTOR vDiffuse = XMVector3Dot(XMLoadFloat3(&vResultNormal), vLightDir);
		FLOAT fDiffuse; 		
		XMStoreFloat(&fDiffuse, vDiffuse);
		fDiffuse = max(fDiffuse, 0.f) * 0.5f + 0.5f;

		// 충돌했다면 그림자를 만들어준다.
		for (auto obj : g_HelloMain->m_ObjectList)
		{
			if (colObj == obj)
				continue;

			Ray tmpRay(vIntersectPos, g_HelloMain->m_pLight->GetPosition());
			XMStoreFloat(&fDist,XMVector3Length(XMLoadFloat3(&vIntersectPos) - XMLoadFloat3(&g_HelloMain->m_pLight->GetPosition())));		

			if (obj->IntersectP(tmpRay, fDist))
			{
				fDiffuse *= 0.7f;
				break;
			}
		}
		
		// Color와 계산하여 Color 데이터에 넣어준다.
		vResultColor = XMFLOAT4(vResultColor.x * fDiffuse, vResultColor.y * fDiffuse, vResultColor.z * fDiffuse, 1.f);
		g_HelloMain->SetPixelColor(m_vPixelPos, vResultColor);
	}
}