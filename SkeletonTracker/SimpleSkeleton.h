#pragma once

#include "Skeleton.h"

#define NUI_SKELETON_POSITION_UNDEFINED -1

typedef
enum _SMPL_SKELETON_POSITION_INDEX
{
	SMPL_SKELETON_POSITION_HIP_CENTER = 0,
	SMPL_SKELETON_POSITION_HIP_RIGHT = (SMPL_SKELETON_POSITION_HIP_CENTER + 1),
	SMPL_SKELETON_POSITION_HIP_LEFT = (SMPL_SKELETON_POSITION_HIP_RIGHT + 1),
	SMPL_SKELETON_POSITION_STOMACH = (SMPL_SKELETON_POSITION_HIP_LEFT + 1),
	SMPL_SKELETON_POSITION_KNEE_RIGHT = (SMPL_SKELETON_POSITION_STOMACH + 1),
	SMPL_SKELETON_POSITION_KNEE_LEFT = (SMPL_SKELETON_POSITION_KNEE_RIGHT + 1),
	SMPL_SKELETON_POSITION_BACKBONE = (SMPL_SKELETON_POSITION_KNEE_LEFT + 1),
	SMPL_SKELETON_POSITION_ANKLE_RIGHT = (SMPL_SKELETON_POSITION_BACKBONE + 1),
	SMPL_SKELETON_POSITION_ANKLE_LEFT = (SMPL_SKELETON_POSITION_ANKLE_RIGHT + 1),
	SMPL_SKELETON_POSITION_CHEST = (SMPL_SKELETON_POSITION_ANKLE_LEFT + 1),
	SMPL_SKELETON_POSITION_FOOT_RIGHT = (SMPL_SKELETON_POSITION_CHEST + 1),
	SMPL_SKELETON_POSITION_FOOT_LEFT = (SMPL_SKELETON_POSITION_FOOT_RIGHT + 1),
	SMPL_SKELETON_POSITION_SHOULDER_CENTER = (SMPL_SKELETON_POSITION_FOOT_LEFT + 1),
	SMPL_SKELETON_POSITION_PECK_RIGHT = (SMPL_SKELETON_POSITION_SHOULDER_CENTER + 1),
	SMPL_SKELETON_POSITION_PECK_LEFT = (SMPL_SKELETON_POSITION_PECK_RIGHT + 1),
	SMPL_SKELETON_POSITION_CHIN = (SMPL_SKELETON_POSITION_PECK_LEFT + 1),
	SMPL_SKELETON_POSITION_SHOULDER_RIGHT = (SMPL_SKELETON_POSITION_CHIN + 1),
	SMPL_SKELETON_POSITION_SHOULDER_LEFT = (SMPL_SKELETON_POSITION_SHOULDER_RIGHT + 1),
	SMPL_SKELETON_POSITION_ELBOW_RIGHT = (SMPL_SKELETON_POSITION_SHOULDER_LEFT + 1),
	SMPL_SKELETON_POSITION_ELBOW_LEFT = (SMPL_SKELETON_POSITION_ELBOW_RIGHT + 1),
	SMPL_SKELETON_POSITION_WRIST_RIGHT = (SMPL_SKELETON_POSITION_ELBOW_LEFT + 1),
	SMPL_SKELETON_POSITION_WRIST_LEFT = (SMPL_SKELETON_POSITION_WRIST_RIGHT + 1),
	SMPL_SKELETON_POSITION_HAND_RIGHT = (SMPL_SKELETON_POSITION_WRIST_LEFT + 1),
	SMPL_SKELETON_POSITION_HAND_LEFT = (SMPL_SKELETON_POSITION_HAND_RIGHT + 1),
	SMPL_SKELETON_POSITION_COUNT = (SMPL_SKELETON_POSITION_HAND_LEFT + 1)
} 	SMPL_SKELETON_POSITION_INDEX;

const int SMPL_INDEX_FROM_KINECT_INDEX[NUI_SKELETON_POSITION_COUNT] = {
	SMPL_SKELETON_POSITION_HIP_CENTER,
	SMPL_SKELETON_POSITION_STOMACH,
	SMPL_SKELETON_POSITION_SHOULDER_CENTER,
	SMPL_SKELETON_POSITION_CHIN,
	SMPL_SKELETON_POSITION_SHOULDER_LEFT,
	SMPL_SKELETON_POSITION_ELBOW_LEFT,
	SMPL_SKELETON_POSITION_WRIST_LEFT,
	SMPL_SKELETON_POSITION_HAND_LEFT,
	SMPL_SKELETON_POSITION_SHOULDER_RIGHT,
	SMPL_SKELETON_POSITION_ELBOW_RIGHT,
	SMPL_SKELETON_POSITION_WRIST_RIGHT,
	SMPL_SKELETON_POSITION_HAND_RIGHT,
	SMPL_SKELETON_POSITION_HIP_LEFT,
	SMPL_SKELETON_POSITION_KNEE_LEFT,
	SMPL_SKELETON_POSITION_ANKLE_LEFT,
	SMPL_SKELETON_POSITION_FOOT_LEFT,
	SMPL_SKELETON_POSITION_HIP_RIGHT,
	SMPL_SKELETON_POSITION_KNEE_RIGHT,
	SMPL_SKELETON_POSITION_ANKLE_RIGHT,
	SMPL_SKELETON_POSITION_FOOT_RIGHT,
};

class SimpleSkeleton : public Skeleton 
{
private:
	Vector4 m_JointsRest[SMPL_SKELETON_POSITION_COUNT];
	Vector4 m_JointsPose[SMPL_SKELETON_POSITION_COUNT];
	D2D1_POINT_2F m_Points[SMPL_SKELETON_POSITION_COUNT];
	const float m_JointThickness;
	const float m_BoneThickness;

public:
	SimpleSkeleton(const float JointThickness = 5.0f, const float BoneThickness = 6.0f);
	void ApplyTransformations(Vector4& scale, Vector4& translate);
	void Render(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pBrushJointSimple,
		ID2D1SolidColorBrush* pBrushBoneSimple, int width, int height);
};