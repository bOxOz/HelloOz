#include "stdafx.h"
#include "Box.h"
#include "Hello.h"
#include "Ray.h"

Box::Box(const XMFLOAT3& vPosition, FLOAT fScale, const XMFLOAT3& vEmittance, const XMFLOAT3& vColor, BOOL bBackCull)
{
	m_vPosition = vPosition;
	m_vScale = XMFLOAT3(fScale, fScale, fScale);

	m_tMaterial.vEmittance = vEmittance;
	m_tMaterial.vBaseColor = vColor;

	if (m_tMaterial.vEmittance.x == 0.f && m_tMaterial.vEmittance.y == 0.f && m_tMaterial.vEmittance.z == 0.f)
		m_tMaterial.bEmitter = FALSE;
	else
		m_tMaterial.bEmitter = TRUE;

	CreateShape(bBackCull);
}

Box::~Box()
{
}

void Box::CreateShape(BOOL bBackCull)
{
	// Create the vertex buffer.
	{
		FLOAT fBoxScale = m_vScale.x * 0.5f;

		Vertex arrVertex[]
		{
			// -Z
			{ { -fBoxScale, fBoxScale, -fBoxScale },	{ 0.f, 0.f, -1.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		{ 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, 0.f, -1.f } },
			
			{ { -fBoxScale, fBoxScale, -fBoxScale },	{ 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, 0.f, -1.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, 0.f, -1.f } },
			
			// Z
			{ { -fBoxScale, -fBoxScale, fBoxScale },	{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 0.f, 1.f } },
			
			{ { -fBoxScale, -fBoxScale, fBoxScale },	{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 0.f, 1.f } },
			{ { -fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 0.f, 1.f } },
			
			// -X
			{ { -fBoxScale, fBoxScale, fBoxScale },		{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, fBoxScale, -fBoxScale },	{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	{ -1.f, 0.f, 0.f } },
			
			{ { -fBoxScale, fBoxScale, fBoxScale },		{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	{ -1.f, 0.f, 0.f } },
			
			// X
			{ { fBoxScale, fBoxScale, -fBoxScale },		{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		{ 1.f, 0.f, 0.f } },
			
			{ { fBoxScale, fBoxScale, -fBoxScale },		{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	{ 1.f, 0.f, 0.f } },
			
			// -Y
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		{ 0.f, -1.f, 0.f } },
			
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		{ 0.f, -1.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	{ 0.f, -1.f, 0.f } },

			// Y
			//{ { -fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 1.f, 0.f } },
			//{ { fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 1.f, 0.f } },
			//{ { fBoxScale, fBoxScale, -fBoxScale },		{ 0.f, 1.f, 0.f } },
			//
			//{ { -fBoxScale, fBoxScale, fBoxScale },		{ 0.f, 1.f, 0.f } },
			//{ { fBoxScale, fBoxScale, -fBoxScale },		{ 0.f, 1.f, 0.f } },
			//{ { -fBoxScale, fBoxScale, -fBoxScale },	{ 0.f, 1.f, 0.f } }
		};

		for (auto vtx : arrVertex)
			m_arrVertex.emplace_back(vtx);

		m_nVtxCnt = m_arrVertex.size();

		if (bBackCull == FALSE)
		{
			for (size_t idx = 0; idx < m_nVtxCnt; ++idx)
			{
				// pos
				if (idx % 3 == 0)
				{
					D3DXVECTOR3 temp = m_arrVertex[idx].vPosition;
					m_arrVertex[idx].vPosition = m_arrVertex[idx + 2].vPosition;
					m_arrVertex[idx + 2].vPosition = temp;
				}

				// norm
				m_arrVertex[idx].vNormal.x *= -1.f;
				m_arrVertex[idx].vNormal.y *= -1.f;
				m_arrVertex[idx].vNormal.z *= -1.f;
			}
		}
	}
}

BOOL Box::Intersect(const Ray& ray, FLOAT& fIntersectDist, XMFLOAT3& vIntersectPos, XMFLOAT3& vIntersectNorm)
{
	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &ray.m_vOrigin, sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &ray.m_vDirection, sizeof(D3DXVECTOR3));

	FLOAT fDist = 0.f, fMinDist = 1000.f;

	for (size_t i = 0; i < m_nVtxCnt; i += 3)
	{
		// 후면은 검사하지 않는다.
		if (D3DXVec3Dot(&m_arrVertex[i].vNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&m_arrVertex[i].vPosition, &m_arrVertex[i + 1].vPosition, &m_arrVertex[i + 2].vPosition, &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			if (fDist < fMinDist)
			{
				fMinDist = fDist;

				D3DXVECTOR3 resPos = m_arrVertex[i].vPosition + (m_arrVertex[i + 1].vPosition - m_arrVertex[i].vPosition) * fU + (m_arrVertex[i + 2].vPosition - m_arrVertex[i].vPosition) * fV;
				vIntersectPos = XMFLOAT3(resPos.x, resPos.y, resPos.z);
				vIntersectNorm = XMFLOAT3(m_arrVertex[i].vNormal.x, m_arrVertex[i].vNormal.y, m_arrVertex[i].vNormal.z);
			}
		}
	}

	if (fMinDist < fIntersectDist)
	{
		fIntersectDist = fMinDist;
		return TRUE;
	}

	return FALSE;
}

BOOL Box::IntersectP(const Ray& ray, FLOAT& fIntersectDist)
{
	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &ray.m_vOrigin, sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &ray.m_vDirection, sizeof(D3DXVECTOR3));

	FLOAT fDist = 0.f, fMinDist = fIntersectDist;

	for (size_t i = 0; i < m_nVtxCnt; i += 3)
	{
		// 후면은 검사하지 않는다.
		if (D3DXVec3Dot(&m_arrVertex[i].vNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&m_arrVertex[i].vPosition, &m_arrVertex[i + 1].vPosition, &m_arrVertex[i + 2].vPosition, &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
			}
		}
	}

	if (fMinDist < fIntersectDist)
	{
		fIntersectDist = fMinDist;
		return TRUE;
	}

	return FALSE;
}