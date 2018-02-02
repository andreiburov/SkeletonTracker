#include "stdafx.h"
#include "SimpleHierarchy.h"
#include "Utils.h"

SimpleHierarchy::SimpleHierarchy()
{
	// Taken from SMPL model for man
	/*DirectX::XMFLOAT4 joints[SMPL_SKELETON_POSITION_COUNT] = {
		{ -0.00087631f, -0.21141872f, 0.02782112f, 1.0f},
		{ 0.07048489f, -0.30100253f, 0.01977493f, 1.0f},
		{ -0.06988833f, -0.30037916f, 0.02302543f, 1.0f},
		{ -0.00338452f, -0.10816186f, 0.00563598f, 1.0f},
		{ 0.10115381f, -0.6652119f, 0.01308602f, 1.0f},
		{ -0.10604072f, -0.67102962f, 0.01384011f, 1.0f},
		{ 0.00019644f, 0.01949579f, 0.00392297f, 1.0f},
		{ 0.08959991f, -1.04856032f, -0.03041559f, 1.0f},
		{ -0.09201208f, -1.05466743f, -0.02805149f, 1.0f},
		{ 0.00222362f, 0.06856801f, 0.03179018f, 1.0f},
		{ 0.11293758f, -1.10320516f, 0.08395453f, 1.0f},
		{ -0.1140553f, -1.10107698f, 0.08984822f, 1.0f},
		{ 0.00026099f, 0.2768112f, -0.0179753f, 1.0f},
		{ 0.0775219f, 0.18634844f, -0.00508464f, 1.0f},
		{ -0.0748092f, 0.18417421f, -0.01002048f, 1.0f},
		{ 0.00377815f, 0.33913339f, 0.03222996f, 1.0f},
		{ 0.16283901f, 0.21808746f, -0.01237748f, 1.0f},
		{ -0.16401207f, 0.21695904f, -0.01982267f, 1.0f},
		{ 0.41408632f, 0.20612068f, -0.03989592f, 1.0f},
		{ -0.41000173f, 0.20380668f, -0.03998439f, 1.0f},
		{ 0.65210542f, 0.21512755f, -0.03985218f, 1.0f},
		{ -0.65517855f, 0.21242863f, -0.04351591f, 1.0f},
		{ 0.73177317f, 0.20544502f, -0.05305777f, 1.0f},
		{ -0.73557876f, 0.20518065f, -0.05393523f, 1.0f}
	};*/
	DirectX::XMFLOAT4 joints[SMPL_SKELETON_POSITION_COUNT] = {
		{ -0.00217368f, -0.24078917f,  0.02858379f, 1.0f },
		{ 0.05640767f, -0.32306921f,  0.01091971f, 1.0f },
		{ -0.06248341f, -0.33130245f,  0.01504126f, 1.0f },
		{ 0.00226577f, -0.11638562f, -0.00980143f, 1.0f },
		{ 0.0998591f,  -0.70953867f,  0.01895671f, 1.0f },
		{ -0.10574003f, -0.71499033f,  0.01019822f, 1.0f },
		{ 0.00675421f,  0.02157077f,  0.0170189f, 1.0f },
		{ 0.08506877f, -1.13641306f, -0.01847128f, 1.0f },
		{ -0.08668449f, -1.13503592f, -0.02436345f, 1.0f },
		{ 0.00448962f,  0.07760317f,  0.01987394f, 1.0f },
		{ 0.12612313f, -1.19669901f,  0.10357114f, 1.0f },
		{ -0.12152436f, -1.19714148f,  0.10595984f, 1.0f },
		{ -0.00890056f,  0.28923869f, -0.01359363f, 1.0f },
		{ 0.0761921f,   0.19160286f,  0.00097577f, 1.0f },
		{ -0.07846403f,  0.1900755f,  -0.00383344f, 1.0f },
		{ 0.00121265f,  0.37817605f,  0.03681623f, 1.0f },
		{ 0.19911348f,  0.23680795f, -0.01807023f, 1.0f },
		{ -0.19169234f,  0.23692877f, -0.01230551f, 1.0f },
		{ 0.45444538f,  0.22115892f, -0.04101671f, 1.0f },
		{ -0.45181985f,  0.22255948f, -0.04357424f, 1.0f },
		{ 0.72015463f,  0.23385703f, -0.04839145f, 1.0f },
		{ -0.72092823f,  0.2293532f,  -0.04960101f, 1.0f },
		{ 0.80684518f,  0.223221f,   -0.06398574f, 1.0f },
		{ -0.80968199f,  0.22070164f, -0.05970808f, 1.0f }
	};

	for (unsigned short i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		m_Joints[i] = DirectX::XMLoadFloat4(&joints[i]);
	}
}

Eigen::Matrix4d skinPartOne(DirectX::XMVECTOR q, DirectX::XMVECTOR j)
{
	Eigen::Matrix4d t = Eigen::Matrix4d::Zero();
	t.block<3, 3>(0, 0) = util::QUATERNION(
		DirectX::XMVectorGetX(q),
		DirectX::XMVectorGetY(q),
		DirectX::XMVectorGetZ(q),
		DirectX::XMVectorGetW(q)
	).normalized().toRotationMatrix();
	t.rightCols<1>() = Eigen::Vector4d(DirectX::XMVectorGetX(j),
		DirectX::XMVectorGetY(j), DirectX::XMVectorGetZ(j), 1.f);
	return t;
}

Eigen::Matrix4d skinPartTwo(Eigen::Matrix4d r, DirectX::XMVECTOR j)
{
	Eigen::Vector4d ej(DirectX::XMVectorGetX(j),
		DirectX::XMVectorGetY(j), DirectX::XMVectorGetZ(j), 0.f);
	ej = r * ej;
	Eigen::Matrix4d J = Eigen::Matrix4d::Zero();
	J.rightCols<1>() = ej;
	return r - J;
}

void SimpleHierarchy::UpdateWithEigenSmplStyle(const SimpleRotations& rotations, bool traceable)
{
	Eigen::Matrix4d transform[SMPL_SKELETON_POSITION_COUNT];

	transform[0] = skinPartOne(rotations[(_SMPL_SKELETON_POSITION_INDEX)0], m_Joints[0]);

	// hierarchical step
	for (int i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Here can be some low pass filtering
		transform[i] = transform[SMPL_PARENT_INDEX[i]]
			* skinPartOne(rotations[(_SMPL_SKELETON_POSITION_INDEX)i],
				DirectX::XMVectorSubtract(m_Joints[i], m_Joints[SMPL_PARENT_INDEX[i]]));
	}

	if (traceable)
	{
		util::TempFile file("Skinning1");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << transform[i](0, 0) << " " << transform[i](0, 1) << " " << transform[i](0, 2) << " " << transform[i](0, 3) << L"\n";
			file << transform[i](1, 0) << " " << transform[i](1, 1) << " " << transform[i](1, 2) << " " << transform[i](1, 3) << "\n";
			file << transform[i](2, 0) << " " << transform[i](2, 1) << " " << transform[i](2, 2) << " " << transform[i](2, 3) << "\n";
			file << transform[i](3, 0) << " " << transform[i](3, 1) << " " << transform[i](3, 2) << " " << transform[i](3, 3) << "\n";
		}
	}

	// smpl step
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		transform[i] = skinPartTwo(transform[i], m_Joints[i]);
		m_Transform[i] = util::MATRIX(transform[i]);
	}

	if (traceable)
	{
		util::TempFile file("Skinning2");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << transform[i](0, 0) << " " << transform[i](0, 1) << " " << transform[i](0, 2) << " " << transform[i](0, 3) << L"\n";
			file << transform[i](1, 0) << " " << transform[i](1, 1) << " " << transform[i](1, 2) << " " << transform[i](1, 3) << "\n";
			file << transform[i](2, 0) << " " << transform[i](2, 1) << " " << transform[i](2, 2) << " " << transform[i](2, 3) << "\n";
			file << transform[i](3, 0) << " " << transform[i](3, 1) << " " << transform[i](3, 2) << " " << transform[i](3, 3) << "\n";
		}
	}

	if (traceable)
	{
		util::TempFile file("Skinning");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleHierarchy::UpdateWithDirectXSmplStyle(const SimpleRotations& rotations, bool traceable)
{
	DirectX::XMMATRIX transform[SMPL_SKELETON_POSITION_COUNT];

	transform[0] = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1), 
		DirectX::XMVectorSet(0, 0, 0, 1), rotations[SMPL_SKELETON_POSITION_HIP_CENTER], m_Joints[0]);

	// hierarchical step
	for (int i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Here can be some low pass filtering
		transform[i] = transform[SMPL_PARENT_INDEX[i]]
			* DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1),
				DirectX::XMVectorSet(0, 0, 0, 1),
				rotations[(_SMPL_SKELETON_POSITION_INDEX)i], 
				DirectX::XMVectorSubtract(m_Joints[i], m_Joints[SMPL_PARENT_INDEX[i]]));
	}

	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Transpose, because HLSL expects matrices in the column major form
		DirectX::XMStoreFloat4x4(&m_Transform[i], DirectX::XMMatrixTranspose(transform[i]));
	}

	if (traceable)
	{
		util::TempFile file("Skinning1");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}

	// smpl step
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		//DirectX::XMVECTOR j = DirectX::XMVectorSetW(m_Joints[i], 0.f);
		DirectX::XMFLOAT4 j;
		DirectX::XMStoreFloat4(&j, DirectX::XMVector4Transform(
			DirectX::XMVectorSetW(m_Joints[i], 0.f), transform[i]));
		DirectX::XMFLOAT4X4 m;
		m._11 = 0.f;
		m._12 = 0.f;
		m._13 = 0.f;
		m._14 = 0.f;
		m._21 = 0.f;
		m._22 = 0.f;
		m._23 = 0.f;
		m._24 = 0.f;
		m._31 = 0.f;
		m._32 = 0.f;
		m._33 = 0.f;
		m._34 = 0.f;
		m._41 = j.x;
		m._42 = j.y;
		m._43 = j.z;
		m._44 = j.w;
		transform[i] = transform[i] - DirectX::XMLoadFloat4x4(&m);
	}

	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Transpose, because HLSL expects matrices in the column major form
		DirectX::XMStoreFloat4x4(&m_Transform[i], DirectX::XMMatrixTranspose(transform[i]));
	}

	if (traceable)
	{
		util::TempFile file("Skinning");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleHierarchy::UpdateWithDirectX(const SimpleRotations& rotations, bool traceable)
{
	DirectX::XMMATRIX transform[SMPL_SKELETON_POSITION_COUNT];

	transform[0] = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1),
		m_Joints[0], rotations[SMPL_SKELETON_POSITION_HIP_CENTER], DirectX::XMVectorSet(0, 0, 0, 1));

	// Update children transformations
	for (int i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Here can be some low pass filtering
		transform[i] = transform[SMPL_PARENT_INDEX[i]]
			* DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1),
				DirectX::XMVectorSubtract(m_Joints[i], m_Joints[SMPL_PARENT_INDEX[i]]),
				rotations[(_SMPL_SKELETON_POSITION_INDEX)i], 
				DirectX::XMVectorSet(0, 0, 0, 1));
	}

	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Transpose, because HLSL expects matrices in the column major form
		DirectX::XMStoreFloat4x4(&m_Transform[i], DirectX::XMMatrixTranspose(transform[i]));
	}

	if (traceable)
	{
		util::TempFile file("Skinning");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleHierarchy::UpdateWithEigen(const SimpleRotations& rotations, bool traceable)
{
	Eigen::Matrix4d transform[SMPL_SKELETON_POSITION_COUNT];
	
	transform[0] = util::RotateAroundPoint(rotations[SMPL_SKELETON_POSITION_HIP_CENTER], m_Joints[0]);

	// Update children transformations
	for (int i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Here can be some low pass filtering
		transform[i] = transform[SMPL_PARENT_INDEX[i]]
			* util::RotateAroundPoint(rotations[(_SMPL_SKELETON_POSITION_INDEX)i], DirectX::XMVectorSubtract(m_Joints[i], m_Joints[SMPL_PARENT_INDEX[i]]));
	}

	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Transpose, because HLSL expects matrices in the column major form
		m_Transform[i] = util::MATRIX(transform[i]);
	}

	if (traceable)
	{
		util::TempFile file("Skinning");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}