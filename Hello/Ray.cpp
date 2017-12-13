#include "stdafx.h"
#include "Ray.h"
#include "Hello.h"
#include "Camera.h"
#include "Box.h"
#include "Light.h"

Ray::Ray(const XMFLOAT2& vPixelPos)
	: m_vPixelPos(vPixelPos), m_vDir(XMFLOAT3(0.f, 0.f, 0.f))	// 레이 시작점은 그냥 카메라 위치 가져오지 뭐
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

	// 모든 오브젝트에 대해 모든 면(후면 제외)에 대해 교차 검사
	// 모든 면에 대해 검사하는 것은 지금 박스 오브젝트 밖에 없기 때문에...
	// 나중에 메쉬나 더 복잡한 모델 추가 시에는 충돌 체크에 대해 최적화가 필요하다.
	Box* pBox = g_HelloMain->m_pBox;
	Box* pRoom = g_HelloMain->m_pRoom;

	D3DXVECTOR3 vRayPos, vRayDir;
	memcpy(&vRayPos, &g_HelloMain->GetCamera()->GetPos(), sizeof(D3DXVECTOR3));
	memcpy(&vRayDir, &m_vDir, sizeof(D3DXVECTOR3));

	FLOAT fDist = 1000.f, fMinDist = 1000.f;

	D3DXVECTOR3 vResultPos { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultNormal { 0.f, 0.f, 0.f };
	D3DXVECTOR3 vResultColor{ 0.f, 0.f, 0.f };

	// 박스와의 검사
	for (UINT i = 0; i < pBox->m_arrVertex.size(); i += 3)
	{
		D3DXVECTOR3 vVertex[3];
		memcpy(vVertex[0], &pBox->m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pBox->m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pBox->m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal;
		memcpy(vPlaneNormal, &pBox->m_arrVertex[i].vNormal, sizeof(D3DXVECTOR3));

		// 후면은 검사하지 않는다.
		if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// 레이가 물체와 교차했다면 교차점을 찾는다.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pBox->m_arrVertex[i].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// 방과 검사
	for (UINT i = 0; i < pRoom->m_arrVertex.size(); i += 3)
	{
		D3DXVECTOR3 vVertex[3];
		memcpy(vVertex[0], &pRoom->m_arrVertex[i].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[1], &pRoom->m_arrVertex[i + 1].vPosition, sizeof(D3DXVECTOR3));
		memcpy(vVertex[2], &pRoom->m_arrVertex[i + 2].vPosition, sizeof(D3DXVECTOR3));

		D3DXVECTOR3 vPlaneNormal;
		memcpy(vPlaneNormal, &pRoom->m_arrVertex[i].vNormal, sizeof(D3DXVECTOR3));

		// 후면은 검사하지 않는다.
		if (D3DXVec3Dot(&vPlaneNormal, &vRayDir) > 0.f)
			continue;

		FLOAT fU = 0.f, fV = 0.f;
		if (D3DXIntersectTri(&vVertex[0], &vVertex[1], &vVertex[2], &vRayPos, &vRayDir, &fU, &fV, &fDist))
		{
			// 레이가 물체와 교차했다면 교차점을 찾는다.
			if (fDist < fMinDist)
			{
				fMinDist = fDist;
				vResultPos = vVertex[0] + (vVertex[1] - vVertex[0]) * fU + (vVertex[2] - vVertex[0]) * fV;
				vResultNormal = vPlaneNormal;
				memcpy(&vResultColor, &pRoom->m_arrVertex[i].vColor, sizeof(D3DXVECTOR3));
			}
		}
	}

	// 교차하는 물체를 찾았다면 Light와 계산하여 Diffuse값을 결정
	if (fMinDist < 1000.f)
	{
		D3DXVECTOR3 vLightPos, vLightDir;
		memcpy(&vLightPos, &g_HelloMain->m_pLight->GetPos(), sizeof(D3DXVECTOR3));
		vLightDir = (vResultPos - vLightPos) * -1.f;
		D3DXVec3Normalize(&vLightDir, &vLightDir);

		FLOAT fDiffuse = D3DXVec3Dot(&vResultNormal, &vLightDir);
		fDiffuse = max(fDiffuse, 0.f) * 0.5f + 0.5f;

		// 빛과 한번 더 계산한다.
		// 교차점과 광원 Dir을 통해 다시 Ray를 쐈을 때, 
		// (방에 대해서만 박스에) 충돌했다면 그림자를 만들어준다.
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
					// 레이가 물체와 교차했다면 어둡게 만들고 끝!
					fDiffuse *= 0.7f;
					break;
				}
			}
		}


		// Color와 계산하여 Color 데이터에 넣어준다.
		vResultColor = vResultColor * fDiffuse;
		g_HelloMain->SetPixelColor(XMFLOAT2(m_vPixelPos.x, m_vPixelPos.y), XMFLOAT4(vResultColor.x, vResultColor.y, vResultColor.z, 1.f));
	}
}