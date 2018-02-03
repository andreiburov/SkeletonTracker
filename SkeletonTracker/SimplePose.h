#pragma once

#include "SimpleDefinitions.h"

class SimplePose
{
public:

	SimplePose() {}

	void Update(SimpleRotations rotations, bool traceable);
	void* getPoseData() { return &m_R; }
	unsigned int getByteWidth() { return sizeof(m_R); }

private:

	DirectX::XMFLOAT3 m_thetas[SMPL_SKELETON_POSITION_COUNT - 1];
	DirectX::XMFLOAT3X3 m_R[SMPL_SKELETON_POSITION_COUNT - 1];
};