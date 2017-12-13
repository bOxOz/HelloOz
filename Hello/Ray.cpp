#include "stdafx.h"
#include "Ray.h"
#include "Hello.h"
#include "Camera.h"
#include "Box.h"
#include "Light.h"

Ray::Ray(const XMFLOAT2& vPixelPos)
	: m_vPixelPos(vPixelPos), m_vDir(XMFLOAT3(0.f, 0.f, 0.f))	// ���� �������� �׳� ī�޶� ��ġ �������� ��
{
}

Ray::~Ray()
{

}

void Ray::SetDir()
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

void Ray::IntersectObject()
{
	SetDir();

	// ��� ������Ʈ�� ���� ��� ��(�ĸ� ����)�� ���� ���� �˻�
	// ��� �鿡 ���� �˻��ϴ� ���� ���� �ڽ� ������Ʈ �ۿ� ���� ������...
	// ���߿� �޽��� �� ������ �� �߰� �ÿ��� �浹 üũ�� ���� ����ȭ�� �ʿ��ϴ�.
	Box* pBox = g_HelloMain->m_pBox;
	Box* pRoom = g_HelloMain->m_pRoom;

	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &g_HelloMain->GetCamera()->GetPos(), sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &m_vDir, sizeof(D3DXVECTOR3));

	FLOAT fDist = 1000.f, fMinDist = 1000.f;

	D3DXVECTOR3 vResultPos { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultNormal { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultColor{ 0.f, 0.f, 0.f };

	// �ڽ����� �˻�
	for (UINT i = 0; i < pBox->m_arrVertex.size(); i += 3)
	{
		D3DXVECTOR3 vVertex[3];
		memcpy(vVertex[0], &pBox->m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pBox->m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pBox->m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal;
		memcpy(vPlaneNormal, &pBox->m_arrVertex[i].vNormal, sizeof(D3DXVECTOR3));

		// �ĸ��� �˻����� �ʴ´�.
		if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// ���̰� ��ü�� �����ߴٸ� �������� ã�´�.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pBox->m_arrVertex[i].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// ��� �˻�
	for (UINT i = 0; i < pRoom->m_arrVertex.size(); i += 3)
	{
		D3DXVECTOR3 vVertex[3];
		memcpy(vVertex[0], &pRoom->m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pRoom->m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pRoom->m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal;
		memcpy(vPlaneNormal, &pRoom->m_arrVertex[i].vNormal, sizeof(D3DXVECTOR3));

		// �ĸ��� �˻����� �ʴ´�.
		if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// ���̰� ��ü�� �����ߴٸ� �������� ã�´�.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pRoom->m_arrVertex[i].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// �����ϴ� ��ü�� ã�Ҵٸ� Light�� ����Ͽ� Diffuse���� ����
	if (fMinDist < 1000.f)
	{
		D3DXVECTOR3 vLightPos, vLightDir;
		memcpy(&vLightPos, &g_HelloMain->m_pLight->GetPos(), sizeof(D3DXVECTOR3));
		vLightDir = (vResultPos - vLightPos) * -1.f;
		D3DXVec3Normalize(&vLightDir, &vLightDir);

		FLOAT fDiffuse = D3DXVec3Dot(&vResultNormal, &vLightDir);
		fDiffuse = max(fDiffuse, 0.f) * 0.5f + 0.5f;

		// ���� �ѹ� �� ����Ѵ�.
		// �������� ���� Dir�� ���� �ٽ� Ray�� ���� ��, 
		// (�濡 ���ؼ��� �ڽ���) �浹�ߴٸ� �׸��ڸ� ������ش�.
		if (vResultColor.z > 0.2f)
		{
			D3DXVECTOR3 vVertex[3];

			fDist = 100.f;
			FLOAT fU = 0.f, fV = 0.f;

			for (UINT i = 0; i < pBox->m_arrVertex.size(); i += 3)
			{
				memcpy(vVertex[0], &pBox->m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
				memcpy(vVertex[1], &pBox->m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
				memcpy(vVertex[2], &pBox->m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

				if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vResultPos, &vLightDir, &fU, &fV, &fDist))
				{
					// ���̰� ��ü�� �����ߴٸ� ��Ӱ� ����� ��!
					fDiffuse *= 0.7f;
					break;
				}
			}
		}


		// Color�� ����Ͽ� Color �����Ϳ� �־��ش�.
		vResultColor = vResultColor * fDiffuse;
		g_HelloMain->SetPixelColor(XMFLOAT2(m_vPixelPos.x, m_vPixelPos.y), XMFLOAT4(vResultColor.x, vResultColor.y, vResultColor.z, 1.f));
	}
}