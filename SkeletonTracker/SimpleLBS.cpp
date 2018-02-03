#include "stdafx.h"
#include "SimpleLBS.h"
#include "Utils.h"

SimpleLBS::SimpleLBS()
{
	for (unsigned short i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		Vector4 j = g_SimpleSkeleton[(_SMPL_SKELETON_POSITION_INDEX)i];
		DirectX::XMFLOAT4 joint(j.x, j.y, j.z, j.w);
		m_Joints[i] = DirectX::XMLoadFloat4(&joint);
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

void SimpleLBS::UpdateWithEigenSmplStyle(const SimpleRotations& rotations, bool traceable)
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
		util::TempFile file("Skinning1/");
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
		util::TempFile file("Skinning2/");
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
		util::TempFile file("Skinning/");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleLBS::UpdateWithDirectXSmplStyle(const SimpleRotations& rotations, bool traceable)
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
		util::TempFile file("Skinning1/");
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
		util::TempFile file("Skinning/");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleLBS::UpdateWithDirectX(const SimpleRotations& rotations, bool traceable)
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
		util::TempFile file("Skinning/");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}

void SimpleLBS::UpdateWithEigen(const SimpleRotations& rotations, bool traceable)
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
		util::TempFile file("Skinning/");
		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			file << m_Transform[i]._11 << " " << m_Transform[i]._12 << " " << m_Transform[i]._13 << " " << m_Transform[i]._14 << L"\n";
			file << m_Transform[i]._21 << " " << m_Transform[i]._22 << " " << m_Transform[i]._23 << " " << m_Transform[i]._24 << "\n";
			file << m_Transform[i]._31 << " " << m_Transform[i]._32 << " " << m_Transform[i]._33 << " " << m_Transform[i]._34 << "\n";
			file << m_Transform[i]._41 << " " << m_Transform[i]._42 << " " << m_Transform[i]._43 << " " << m_Transform[i]._44 << "\n";
		}
	}
}