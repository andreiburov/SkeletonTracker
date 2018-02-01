#pragma once

#include "SimpleDefinitions.h"

class SimpleHierarchy
{
public:

	SimpleHierarchy();

	unsigned int getByteWidth() { return sizeof(m_Transform);	}
	void* getHierarchyConstantBuffer() { return &m_Transform; }
	void UpdateWithDirectXSmplStyle(const SimpleRotations& rotations, bool traceable);
	void UpdateWithEigenSmplStyle(const SimpleRotations& rotations, bool traceable);
	void UpdateWithDirectX(const SimpleRotations& rotations, bool traceable);
	void UpdateWithEigen(const SimpleRotations& rotations, bool traceable);

private:

	const int SMPL_PARENT_INDEX[SMPL_SKELETON_POSITION_COUNT] = {
		-1,
		SMPL_SKELETON_POSITION_HIP_CENTER,
		SMPL_SKELETON_POSITION_HIP_CENTER,
		SMPL_SKELETON_POSITION_HIP_CENTER,
		SMPL_SKELETON_POSITION_HIP_RIGHT,
		SMPL_SKELETON_POSITION_HIP_LEFT,
		SMPL_SKELETON_POSITION_STOMACH,
		SMPL_SKELETON_POSITION_KNEE_RIGHT,
		SMPL_SKELETON_POSITION_KNEE_LEFT,
		SMPL_SKELETON_POSITION_BACKBONE,
		SMPL_SKELETON_POSITION_ANKLE_RIGHT,
		SMPL_SKELETON_POSITION_ANKLE_LEFT,
		SMPL_SKELETON_POSITION_CHEST,
		SMPL_SKELETON_POSITION_CHEST,
		SMPL_SKELETON_POSITION_CHEST,
		SMPL_SKELETON_POSITION_SHOULDER_CENTER,
		SMPL_SKELETON_POSITION_PECK_RIGHT,
		SMPL_SKELETON_POSITION_PECK_LEFT,
		SMPL_SKELETON_POSITION_SHOULDER_RIGHT,
		SMPL_SKELETON_POSITION_SHOULDER_LEFT,
		SMPL_SKELETON_POSITION_ELBOW_RIGHT,
		SMPL_SKELETON_POSITION_ELBOW_LEFT,
		SMPL_SKELETON_POSITION_WRIST_RIGHT,
		SMPL_SKELETON_POSITION_WRIST_LEFT
	};

	DirectX::XMFLOAT4X4 m_Transform[SMPL_SKELETON_POSITION_COUNT];
	DirectX::XMVECTOR m_Joints[SMPL_SKELETON_POSITION_COUNT];
};