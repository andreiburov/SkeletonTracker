#include "stdafx.h"
#include "SimplePose.h"
#include "Utils.h"

using namespace DirectX;

void SimplePose::Update(SimpleRotations rotations)
{
	for (unsigned short i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
#ifdef _DEBUG
		XMVECTOR theta = util::AxisAngleFromQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]);
		XMStoreFloat3(&m_thetas[i-1], theta);
#endif _DEBUG
		XMStoreFloat3x3(&m_R[i - 1], XMMatrixTranspose(
			XMMatrixRotationQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]) - XMMatrixIdentity()));
	}
}