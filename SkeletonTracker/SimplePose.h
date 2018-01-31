#pragma once

#include "SimpleSkeleton.h"

class SimplePose
{
public:

	SimplePose() {}

	void Update(SimpleRotations rotations);
	void* getPoseConstantBuffer() { return &m_R; }
	unsigned int getByteWidth() { return sizeof(m_R) + (16-sizeof(m_R)%16); } // ConstantBuffer Must be a multiple of 16

private:

#ifdef _DEBUG
	DirectX::XMFLOAT3 m_thetas[SMPL_SKELETON_POSITION_COUNT - 1];
#endif // _DEBUG

	DirectX::XMFLOAT3X3 m_R[SMPL_SKELETON_POSITION_COUNT - 1];
	char padding[(16 - (sizeof(float)*9*(SMPL_SKELETON_POSITION_COUNT - 1)) % 16)];
};