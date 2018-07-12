#pragma once

class Light
{
public:
	Light(const XMFLOAT3& vPosition);
	~Light();

	const XMFLOAT3& GetPosition() { return m_vPosistion; }

private:
	XMFLOAT3 m_vPosistion;
};