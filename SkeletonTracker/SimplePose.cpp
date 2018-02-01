#include "stdafx.h"
#include "SimplePose.h"
#include "Utils.h"

using namespace DirectX;

void SimplePose::Update(SimpleRotations rotations, bool traceable)
{
	util::TempFile file("Rotations");

	for (unsigned short i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		XMStoreFloat3x3(&m_R[i - 1], XMMatrixTranspose(
			XMMatrixRotationQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]) - XMMatrixIdentity()));
		
		if (traceable)
		{
			file << m_R[i-1]._11 << " " << m_R[i-1]._12 << " " << m_R[i-1]._13 << "\n";
			file << m_R[i-1]._21 << " " << m_R[i-1]._22 << " " << m_R[i-1]._23 << "\n";
			file << m_R[i-1]._31 << " " << m_R[i-1]._32 << " " << m_R[i-1]._33 << "\n";
		}
	}
}