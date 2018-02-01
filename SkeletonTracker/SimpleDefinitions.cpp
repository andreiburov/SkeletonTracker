#include "stdafx.h"
#include "Utils.h"
#include "SimpleDefinitions.h"

using namespace DirectX;

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