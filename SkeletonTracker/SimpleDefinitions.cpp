#include "stdafx.h"
#include "Utils.h"
#include "SimpleDefinitions.h"

using namespace DirectX;

// SimpleRotations encapsulate thetas from SMPL paper

SimpleRotations::SimpleRotations()
{
	ZeroMemory(rotations, sizeof(rotations));
	for (unsigned short i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		rotations[i] = DirectX::XMQuaternionIdentity();
	}
}

DirectX::XMVECTOR SimpleRotations::operator[](_SMPL_SKELETON_POSITION_INDEX i) const
{
	return rotations[i];
}

DirectX::XMVECTOR& SimpleRotations::operator[](_SMPL_SKELETON_POSITION_INDEX i)
{
	return rotations[i];
}

void SimpleRotations::printThetas() const
{
	util::TempFile file("Thetas");
	file << L"[ ";

	int N = SMPL_SKELETON_POSITION_COUNT;
	for (int i = 0; i < N; i++)
	{
		XMFLOAT3 t;
		XMVECTOR theta = util::AxisAngleFromQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]);

		XMStoreFloat3(&t, theta);
		file << t.x << ", " << t.y << ", " << t.z;

		if (i != N-1)
		{
			file << ", ";
		}
		else
		{
			file << " ]\n";
		}
	}
}

// SimpleSkeleton encapsulate joints in rest pose

SimpleSkeleton::SimpleSkeleton()
{
	ZeroMemory(joints, sizeof(joints));
	for (unsigned short i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		std::ifstream in("SimpleModel/smpl_skeleton.obj", std::ios::in);
		if (!in)
		{
			MessageBoxW(NULL, (std::wstring(L"Could not open SimpleModel/smpl_skeleton.obj")).c_str(), L"File error", MB_ICONERROR | MB_OK);
		}

		for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
		{
			std::string vertex;
			in >> vertex >> joints[i].x >> joints[i].y >> joints[i].z;
			joints[i].w = 1.f;
		}
		in.close();
	}
}

Vector4 SimpleSkeleton::operator[](_SMPL_SKELETON_POSITION_INDEX i) const
{
	return joints[i];
}

SimpleSkeleton g_SimpleSkeleton;