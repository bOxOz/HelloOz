#pragma once

class Camera 
{
public:
	Camera(FLOAT fNear, FLOAT fFar, XMFLOAT3& vPosition, XMFLOAT3& vTarget);
	~Camera();

	void SetProj(FLOAT fNear, FLOAT fFar);
	void SetView(XMFLOAT3 vPosition, XMFLOAT3 vTarget);

	const XMFLOAT4X4& GetProj() { return m_Proj; }
	const XMFLOAT4X4& GetView() { return m_View; }
	const XMFLOAT3& GetPosition() { return m_Position; }
	const XMFLOAT3& GetTarget() { return m_Target; }

private:
	XMFLOAT4X4	m_Proj;
	XMFLOAT4X4	m_View;
	XMFLOAT3	m_Position;
	XMFLOAT3	m_Target;
};