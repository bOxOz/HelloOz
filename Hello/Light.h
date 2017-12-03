#pragma once

class Light
{
public:
	Light(const XMFLOAT3& vPos);
	~Light();

	const XMFLOAT3& GetPos() { return m_vPos; }

private:
	XMFLOAT3 m_vPos;
};