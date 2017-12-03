#include "stdafx.h"
#include "Ray.h"
#include "Hello.h"
#include "Camera.h"
#include "Box.h"
#include "Light.h"

Ray::Ray(const XMFLOAT2& vPixelPos)
	: m_vPixelPos(vPixelPos), m_vNormal(XMFLOAT3(0.f, 0.f, 0.f))	// 레이 시작점은 그냥 카메라 위치 가져오지 뭐
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
	// 모든 오브젝트에 대해 모든 면(후면 제외)에 대해 교차 검사
	// 모든 면에 대해 검사하는 것은 지금 박스 오브젝트 밖에 없기 때문에...
	// 나중에 메쉬나 더 복잡한 모델 추가 시에는 이 부분에 대해 최적화가 필요하다.
	Box* pBox = g_HelloMain->m_pBox;
	Box* pRoom = g_HelloMain->m_pRoom;

	D3DXVECTOR3 vVertex[3], vNormal[3];

	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &g_HelloMain->GetCamera()->GetPos(), sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &m_vNormal, sizeof(D3DXVECTOR3));

	// 박스와의 검사
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
		// 후면은 검사하지 않는다.
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
			// 레이가 물체와 교차했다면 교차점을 찾는다.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pVtx[pIdx[i]].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// 방과 검사
	memcpy(pVtx, pRoom->m_arrVertices, sizeof(Vertex) * 8);
	memcpy(pIdx, pRoom->m_arrIndeces, sizeof(UINT) * 36);

	for (UINT i = 0; i < pRoom->m_nIdxCnt; i += 3)
	{
		// 후면은 검사하지 않는다.
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
			// 레이가 물체와 교차했다면 교차점을 찾는다.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pVtx[pIdx[i]].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// 교차하는 물체를 찾았다면 Light와 계산하여 Diffuse값을 결정
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

		// Color와 계산하여 Color 데이터에 넣어준다.
		vResultColor = vResultColor * fDiffuse;
		g_HelloMain->SetPixelColor(XMFLOAT2(vResultPos.x, vResultPos.y), XMFLOAT3(vResultColor.x, vResultColor.y, vResultColor.z));
	}
}