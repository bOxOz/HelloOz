#pragma once

class Camera 
{
public:
	Camera(FLOAT fNear, FLOAT fFar, XMFLOAT3& vPos, XMFLOAT3& vTarget);
	~Camera();

	void SetProj(FLOAT fNear, FLOAT fFar);
	void SetView(XMFLOAT3 vPos, XMFLOAT3 vTarget);

	const XMFLOAT4X4& GetProj() { return m_Proj; }
	const XMFLOAT4X4& GetView() { return m_View; }

private:
	XMFLOAT4X4	m_Proj;
	XMFLOAT4X4	m_View;
	XMFLOAT3	m_Pos;
	XMFLOAT3	m_Target;
};