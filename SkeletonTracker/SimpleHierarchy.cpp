#include "stdafx.h"
#include "SimpleHierarchy.h"

SimpleHierarchy::SimpleHierarchy()
{
	// Taken from SMPL model for man
	DirectX::XMFLOAT4 joints[SMPL_SKELETON_POSITION_COUNT] = {
		{ -0.00087631f, -0.21141872f, 0.02782112f, 1.0f},
		{ 0.07048489f, -0.30100253f, 0.01977493f, 1.0f},
		{ -0.06988833f, -0.30037916f, 0.02302543f, 1.0f},
		{ -0.00338452f, -0.10816186f, 0.00563598f, 1.0f},
		{ 0.10115381f, -0.6652119f, 0.01308602f, 1.0f},
		{ -0.10604072f, -0.67102962f, 0.01384011f, 1.0f},
		{ 0.00019644f, 0.01949579f, 0.00392297f, 1.0f},
		{ 0.08959991f, -1.04856032f, -0.03041559f, 1.0f},
		{ -0.09201208f, -1.05466743f, -0.02805149f, 1.0f},
		{ 0.00222362f, 0.06856801f, 0.03179018f, 1.0f},
		{ 0.11293758f, -1.10320516f, 0.08395453f, 1.0f},
		{ -0.1140553f, -1.10107698f, 0.08984822f, 1.0f},
		{ 0.00026099f, 0.2768112f, -0.0179753f, 1.0f},
		{ 0.0775219f, 0.18634844f, -0.00508464f, 1.0f},
		{ -0.0748092f, 0.18417421f, -0.01002048f, 1.0f},
		{ 0.00377815f, 0.33913339f, 0.03222996f, 1.0f},
		{ 0.16283901f, 0.21808746f, -0.01237748f, 1.0f},
		{ -0.16401207f, 0.21695904f, -0.01982267f, 1.0f},
		{ 0.41408632f, 0.20612068f, -0.03989592f, 1.0f},
		{ -0.41000173f, 0.20380668f, -0.03998439f, 1.0f},
		{ 0.65210542f, 0.21512755f, -0.03985218f, 1.0f},
		{ -0.65517855f, 0.21242863f, -0.04351591f, 1.0f},
		{ 0.73177317f, 0.20544502f, -0.05305777f, 1.0f},
		{ -0.73557876f, 0.20518065f, -0.05393523f, 1.0f}
	};

	for (unsigned short i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		m_Joints[i] = DirectX::XMLoadFloat4(&joints[i]);
	}
}

void SimpleHierarchy::Update(const SimpleRotations& rotations)
{
	DirectX::XMMATRIX transform[SMPL_SKELETON_POSITION_COUNT];
	transform[0] = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1), 
		m_Joints[0], rotations[SMPL_SKELETON_POSITION_HIP_CENTER], DirectX::XMVectorSet(0, 0, 0, 1));

	// Update children transformations
	for (int i = SMPL_SKELETON_POSITION_HIP_RIGHT; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Here can be some low pass filtering
		transform[i] = transform[SMPL_PARENT_INDEX[i]]
			* DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1, 1, 1, 1),
				DirectX::XMVectorSubtract(m_Joints[i], m_Joints[SMPL_PARENT_INDEX[i]]),
				rotations[(_SMPL_SKELETON_POSITION_INDEX)i], DirectX::XMVectorSet(0, 0, 0, 1));
	}

	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		// Transpose, because HLSL expects matrices in the column major form
		DirectX::XMStoreFloat4x4(&m_Transform[i], DirectX::XMMatrixTranspose(transform[i]));
	}
}