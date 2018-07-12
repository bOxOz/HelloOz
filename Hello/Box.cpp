#include "stdafx.h"
#include "Box.h"
#include "Hello.h"
#include "Ray.h"

Box::Box(const XMFLOAT3& vPosition, FLOAT fScale, const XMFLOAT4& vColor, BOOL bBackCull)
{
	m_vPosition = vPosition;
	m_vScale = XMFLOAT3(fScale, fScale, fScale);
	m_vColor = vColor;

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
			{ { -fBoxScale, fBoxScale, -fBoxScale },	m_vColor, { 0.f, 0.f, -1.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		m_vColor, { 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	m_vColor, { 0.f, 0.f, -1.f } },

			{ { -fBoxScale, fBoxScale, -fBoxScale },	m_vColor,{ 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ 0.f, 0.f, -1.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	m_vColor, { 0.f, 0.f, -1.f } },
			
			// Z
			{ { -fBoxScale, -fBoxScale, fBoxScale },	m_vColor,{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		m_vColor,{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 0.f, 0.f, 1.f } },

			{ { -fBoxScale, -fBoxScale, fBoxScale },	m_vColor,{ 0.f, 0.f, 1.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 0.f, 0.f, 1.f } },
			{ { -fBoxScale, fBoxScale, fBoxScale },		m_vColor, { 0.f, 0.f, 1.f } },

			// -X
			{ { -fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, fBoxScale, -fBoxScale },	m_vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ -1.f, 0.f, 0.f } },

			{ { -fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	m_vColor,{ -1.f, 0.f, 0.f } },

			// X
			{ { fBoxScale, fBoxScale, -fBoxScale },		m_vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		m_vColor,{ 1.f, 0.f, 0.f } },

			{ { fBoxScale, fBoxScale, -fBoxScale },		m_vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		m_vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ 1.f, 0.f, 0.f } },

			// -Y
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		m_vColor,{ 0.f, -1.f, 0.f } },

			{ { -fBoxScale, -fBoxScale, -fBoxScale },	m_vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		m_vColor,{ 0.f, -1.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	m_vColor,{ 0.f, -1.f, 0.f } },

			// Y
			{ { -fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		m_vColor,{ 0.f, 1.f, 0.f } },

			{ { -fBoxScale, fBoxScale, fBoxScale },		m_vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		m_vColor,{ 0.f, 1.f, 0.f } },
			{ { -fBoxScale, fBoxScale, -fBoxScale },	m_vColor,{ 0.f, 1.f, 0.f } }
		};

		for (auto vtx : arrVertex)
			m_arrVertex.emplace_back(vtx);

		if (bBackCull == FALSE)
		{
			for (INT idx = 0; idx < m_arrVertex.size(); ++idx)
			{
				// pos
				if (idx % 3 == 0)
				{
					XMFLOAT3 temp = m_arrVertex[idx].vPosition;
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

	FLOAT fDist = 0.f, fMinDist = fIntersectDist;

	for (UINT i = 0; i < m_arrVertex.size(); i += 3)
	{
		D3DXVECTOR3 vVertex[3];
		memcpy(vVertex[0], &m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal;
		memcpy(vPlaneNormal, &m_arrVertex[i].vNormal, sizeof(D3DXVECTOR3));

		// 후면은 검사하지 않는다.
		if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			if (fDist < fMinDist)
			{
				fMinDist = fDist;

				D3DXVECTOR3 resPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vIntersectPos = XMFLOAT3(resPos.x, resPos.y, resPos.z);
				vIntersectNorm = XMFLOAT3(vPlaneNormal.x, vPlaneNormal.y, vPlaneNormal.z);
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