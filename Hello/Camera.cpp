#include "stdafx.h"
#include "Camera.h"
#include "Hello.h"

Camera::Camera(FLOAT fNear, FLOAT fFar, XMFLOAT3& vPos, XMFLOAT3& vTarget)
	: m_Pos(0.f, 0.f, 0.f), m_Target(0.f, 0.f, 1.f)
{
	XMStoreFloat4x4(&m_Proj, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());

	SetProj(fNear, fFar);
	SetView(vPos, vTarget);
}

Camera::~Camera()
{

}

void Camera::SetProj(FLOAT fNear, FLOAT fFar)
{
	XMMATRIX matProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(35.f), FLOAT(g_HelloMain->GetWidth()) / FLOAT(g_HelloMain->GetHeight()), fNear, fFar);
	XMStoreFloat4x4(&m_Proj, matProj);
}

void Camera::SetView(XMFLOAT3 vPos, XMFLOAT3 vTarget)
{
	m_Pos = vPos;
	m_Target = vTarget;

	XMMATRIX matView = XMMatrixLookAtLH(XMLoadFloat3(&m_Pos), XMLoadFloat3(&m_Target), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	XMStoreFloat4x4(&m_View, matView);
}