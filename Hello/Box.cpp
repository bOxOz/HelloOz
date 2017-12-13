#include "stdafx.h"
#include "Box.h"
#include "Hello.h"

Box::Box()
{
}

Box::~Box()
{
}

void Box::CreateShape(FLOAT fScale, const XMFLOAT4& vColor, BOOL bBackCull)
{
	// Create the vertex buffer.
	{
		FLOAT fBoxScale = fScale * 0.5f;

		Vertex arrVertex[]
		{
			// -Z
			{ { -fBoxScale, fBoxScale, -fBoxScale },	vColor, { 0.f, 0.f, -1.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		vColor, { 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	vColor, { 0.f, 0.f, -1.f } },

			{ { -fBoxScale, fBoxScale, -fBoxScale },	vColor,{ 0.f, 0.f, -1.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ 0.f, 0.f, -1.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	vColor, { 0.f, 0.f, -1.f } },
			
			// Z
			{ { -fBoxScale, -fBoxScale, fBoxScale },	vColor,{ 0.f, 0.f, 1.f } },	
			{ { fBoxScale, -fBoxScale, fBoxScale },		vColor,{ 0.f, 0.f, 1.f } },	
			{ { fBoxScale, fBoxScale, fBoxScale },		vColor,{ 0.f, 0.f, 1.f } },	

			{ { -fBoxScale, -fBoxScale, fBoxScale },	vColor,{ 0.f, 0.f, 1.f } },	
			{ { fBoxScale, fBoxScale, fBoxScale },		vColor,{ 0.f, 0.f, 1.f } },	
			{ { -fBoxScale, fBoxScale, fBoxScale },		vColor, { 0.f, 0.f, 1.f } },

			// -X
			{ { -fBoxScale, fBoxScale, fBoxScale },		vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, fBoxScale, -fBoxScale },	vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ -1.f, 0.f, 0.f } },

			{ { -fBoxScale, fBoxScale, fBoxScale },		vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ -1.f, 0.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	vColor,{ -1.f, 0.f, 0.f } },

			// X
			{ { fBoxScale, fBoxScale, -fBoxScale },		vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		vColor,{ 1.f, 0.f, 0.f } },

			{ { fBoxScale, fBoxScale, -fBoxScale },		vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		vColor,{ 1.f, 0.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ 1.f, 0.f, 0.f } },

			// -Y
			{ { -fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		vColor,{ 0.f, -1.f, 0.f } },

			{ { -fBoxScale, -fBoxScale, -fBoxScale },	vColor,{ 0.f, -1.f, 0.f } },
			{ { fBoxScale, -fBoxScale, fBoxScale },		vColor,{ 0.f, -1.f, 0.f } },
			{ { -fBoxScale, -fBoxScale, fBoxScale },	vColor,{ 0.f, -1.f, 0.f } },

			// Y
			{ { -fBoxScale, fBoxScale, fBoxScale },		vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, fBoxScale },		vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		vColor,{ 0.f, 1.f, 0.f } },

			{ { -fBoxScale, fBoxScale, fBoxScale },		vColor,{ 0.f, 1.f, 0.f } },
			{ { fBoxScale, fBoxScale, -fBoxScale },		vColor,{ 0.f, 1.f, 0.f } },
			{ { -fBoxScale, fBoxScale, -fBoxScale },	vColor,{ 0.f, 1.f, 0.f } }
		};

		for (auto vtx : arrVertex)
			m_arrVertex.emplace_back(vtx);

		if (bBackCull == FALSE)
		{
			for (INT idx = 0; idx < m_arrVertex.size(); ++idx)
			{
				// pos
				if (idx % 3 == 0)
				{
					XMFLOAT3 temp = m_arrVertex[idx].vPosition;
					m_arrVertex[idx].vPosition = m_arrVertex[idx + 2].vPosition;
					m_arrVertex[idx + 2].vPosition = temp;
				}

				// nor
				m_arrVertex[idx].vNormal.x *= -1.f;
				m_arrVertex[idx].vNormal.y *= -1.f;
				m_arrVertex[idx].vNormal.z *= -1.f;
			}
		}
	}
}