#pragma once

class Box
{
public:
	Box();
	~Box();

	void CreateShape(FLOAT fScale, const XMFLOAT4& vColor, BOOL bBackCull = TRUE);

	std::vector<Vertex>		m_arrVertex;
};