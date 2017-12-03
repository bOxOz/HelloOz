#include "stdafx.h"
#include "Ray.h"
#include "Hello.h"
#include "Camera.h"
#include "Box.h"
#include "Light.h"

Ray::Ray(const XMFLOAT2& vPixelPos)
	: m_vPixelPos(vPixelPos), m_vNormal(XMFLOAT3(0.f, 0.f, 0.f))	// ���� �������� �׳� ī�޶� ��ġ �������� ��
{
	SetNormal();
}

Ray::~Ray()
{

}

void Ray::SetNormal()
{
	const XMFLOAT4X4& matProj = g_HelloMain->GetCamera()->GetProj();
	m_vNormal.x = ((m_vPixelPos.x / (WINSIZEX >> 1)) - 1.f) / matProj._11;
	m_vNormal.y = ((-m_vPixelPos.y / (WINSIZEY >> 1)) + 1.f) / matProj._22;
	m_vNormal.z = 1.f;

	const XMFLOAT4X4& matView = g_HelloMain->GetCamera()->GetView();
	XMMATRIX matViewInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&matView));
	XMVECTOR vRayNormal = XMVector3Normalize(XMLoadFloat3(&m_vNormal));
	vRayNormal = XMVector3TransformNormal(vRayNormal, matViewInv);

	XMStoreFloat3(&m_vNormal, vRayNormal);
}

void Ray::IntersectObject()
{
	// ��� ������Ʈ�� ���� ��� ��(�ĸ� ����)�� ���� ���� �˻�
	// ��� �鿡 ���� �˻��ϴ� ���� ���� �ڽ� ������Ʈ �ۿ� ���� ������...
	// ���߿� �޽��� �� ������ �� �߰� �ÿ��� �� �κп� ���� ����ȭ�� �ʿ��ϴ�.
	Box* pBox = g_HelloMain->m_pBox;
	Box* pRoom = g_HelloMain->m_pRoom;

	D3DXVECTOR3 vVertex[3], vNormal[3];

	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &g_HelloMain->GetCamera()->GetPos(), sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &m_vNormal, sizeof(D3DXVECTOR3));

	// �ڽ����� �˻�
	Vertex pVtx[8]; 
	memcpy(pVtx, pBox->m_arrVertices, sizeof(Vertex) * 8);

	UINT pIdx[36];
	memcpy(pIdx, pBox->m_arrIndeces, sizeof(UINT) * 36);

	FLOAT fDist = 1000.f, fMinDist = 1000.f;
	FLOAT fU = 0.f, fV = 0.f;
	D3DXVECTOR3 vResultPos { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultNormal { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultColor { 0.f, 0.f, 0.f };
	
	for (UINT i = 0; i < pBox->m_nIdxCnt; i += 3)
	{
		// �ĸ��� �˻����� �ʴ´�.
		memcpy(vNormal[0], &pVtx[pIdx[i]].vNormal, sizeof(D3DXVECTOR3));
		memcpy(vNormal[1], &pVtx[pIdx[i + 1]].vNormal, sizeof(D3DXVECTOR3));
		memcpy(vNormal[2], &pVtx[pIdx[i + 2]].vNormal, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal = vNormal[0] + vNormal[1] + vNormal[2];
		D3DXVec3Normalize(&vPlaneNormal, &vPlaneNormal);

		//if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) < 0.f)
		//	continue;

		memcpy(vVertex[0], &pVtx[pIdx[i]].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pVtx[pIdx[i + 1]].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pVtx[pIdx[i + 2]].vPosition, sizeof(D3DXVECTOR3));

		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// ���̰� ��ü�� �����ߴٸ� �������� ã�´�.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pVtx[pIdx[i]].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// ��� �˻�
	memcpy(pVtx, pRoom->m_arrVertices, sizeof(Vertex) * 8);
	memcpy(pIdx, pRoom->m_arrIndeces, sizeof(UINT) * 36);

	for (UINT i = 0; i < pRoom->m_nIdxCnt; i += 3)
	{
		// �ĸ��� �˻����� �ʴ´�.
		memcpy(vNormal[0], &pVtx[pIdx[i]].vNormal, sizeof(D3DXVECTOR3));
		memcpy(vNormal[1], &pVtx[pIdx[i + 1]].vNormal, sizeof(D3DXVECTOR3));
		memcpy(vNormal[2], &pVtx[pIdx[i + 2]].vNormal, sizeof(D3DXVECTOR3));
		
		D3DXVECTOR3 vPlaneNormal = vNormal[0] + vNormal[1] + vNormal[2];
		D3DXVec3Normalize(&vPlaneNormal, &vPlaneNormal);
		
		//if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) < 0.f)
		//	continue;

		memcpy(vVertex[0], &pVtx[pIdx[i]].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pVtx[pIdx[i + 1]].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pVtx[pIdx[i + 2]].vPosition, sizeof(D3DXVECTOR3));

		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// ���̰� ��ü�� �����ߴٸ� �������� ã�´�.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pVtx[pIdx[i]].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// �����ϴ� ��ü�� ã�Ҵٸ� Light�� ����Ͽ� Diffuse���� ����
	if (fMinDist < 1000.f)
	{
		D3DXVECTOR3 vLightPos, vLightDir;
		memcpy(&vLightPos, &g_HelloMain->m_pLight->GetPos(), sizeof(D3DXVECTOR3));
		vLightDir = vResultPos - vLightPos;
		D3DXVec3Normalize(&vLightDir, &vLightDir);

		FLOAT fDiffuse = D3DXVec3Dot(&vResultNormal, &vLightDir);
		fDiffuse = max(fDiffuse, 0.f);

		D3DXMATRIX matView, matProj, matViewport;
		memcpy(&matView, &g_HelloMain->GetCamera()->GetView(), sizeof(D3DXMATRIX));
		memcpy(&matProj, &g_HelloMain->GetCamera()->GetProj(), sizeof(D3DXMATRIX));
		
		D3DXVec3TransformCoord(&vResultPos, &vResultPos, &matView);
		D3DXVec3TransformCoord(&vResultPos, &vResultPos, &matProj);

		D3DXMatrixIdentity(&matViewport);
		matViewport._11 = FLOAT(WINSIZEX) * 0.5f;
		matViewport._22 = FLOAT(WINSIZEY) * -0.5f;
		matViewport._41 = FLOAT(WINSIZEX) * 0.5f;
		matViewport._42 = FLOAT(WINSIZEY) * 0.5f;

		D3DXVec3TransformCoord(&vResultPos, &vResultPos, &matViewport);

		// Color�� ����Ͽ� Color �����Ϳ� �־��ش�.
		vResultColor = vResultColor * fDiffuse;
		g_HelloMain->SetPixelColor(XMFLOAT2(vResultPos.x, vResultPos.y), XMFLOAT3(vResultColor.x, vResultColor.y, vResultColor.z));
	}
}