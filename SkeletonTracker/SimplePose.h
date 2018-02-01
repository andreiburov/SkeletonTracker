#pragma once

#include "SimpleDefinitions.h"

class SimplePose
{
public:

	SimplePose() {}

	void Update(SimpleRotations rotations, bool traceable);
	void* getPoseConstantBuffer() { return &m_R; }
	unsigned int getByteWidth() { return sizeof(m_R) + (16-sizeof(m_R)%16); } // ConstantBuffer Must be a multiple of 16

private:

	DirectX::XMFLOAT3 m_thetas[SMPL_SKELETON_POSITION_COUNT - 1];
	DirectX::XMFLOAT3X3 m_R[SMPL_SKELETON_POSITION_COUNT - 1];
	byte padding[(16 - (sizeof(float)*9*(SMPL_SKELETON_POSITION_COUNT - 1)) % 16)];
};