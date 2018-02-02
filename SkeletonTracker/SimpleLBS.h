#pragma once

#include "SimpleDefinitions.h"

class SimpleLBS
{
public:

	SimpleLBS();

	unsigned int getByteWidth() { return sizeof(m_Transform);	}
	void* getHierarchyConstantBuffer() { return &m_Transform; }
	void UpdateWithDirectXSmplStyle(const SimpleRotations& rotations, bool traceable);
	void UpdateWithEigenSmplStyle(const SimpleRotations& rotations, bool traceable);
	void UpdateWithDirectX(const SimpleRotations& rotations, bool traceable);
	void UpdateWithEigen(const SimpleRotations& rotations, bool traceable);

private:

	DirectX::XMFLOAT4X4 m_Transform[SMPL_SKELETON_POSITION_COUNT];
	DirectX::XMVECTOR m_Joints[SMPL_SKELETON_POSITION_COUNT]; // Joints to operate on
};