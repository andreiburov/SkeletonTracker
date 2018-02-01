#include "stdafx.h"
#include "SimplePose.h"
#include "Utils.h"

using namespace DirectX;

void SimplePose::Update(SimpleRotations rotations, bool traceable)
{
	for (unsigned short i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		XMStoreFloat3x3(&m_R[i - 1], XMMatrixTranspose(
			XMMatrixRotationQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]) - XMMatrixIdentity()));
		
		if (traceable)
		{
			util::TempFile file("Rotations");
			XMVECTOR theta = util::AxisAngleFromQuaternion(rotations[(SMPL_SKELETON_POSITION_INDEX)i]);
			XMStoreFloat3(&m_thetas[i - 1], theta);
			
			file << L"[ ";
			int n = SMPL_SKELETON_POSITION_COUNT - 1;
			for (int i = 0; i < n; i++)
			{
				file << m_thetas[i].x << L", " << m_thetas[i].y << L", " << m_thetas[i].z << L", ";
			}
			file << m_thetas[n].x << L", " << m_thetas[n].y << L", " << m_thetas[n].z << L" ]\n";

			for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
			{
				file << m_R->_11 << " " << m_R->_12 << " " << m_R->_13 << "\n";
				file << m_R->_21 << " " << m_R->_22 << " " << m_R->_23 << "\n";
				file << m_R->_31 << " " << m_R->_32 << " " << m_R->_33 << "\n";
			}
		}
		
	}
}