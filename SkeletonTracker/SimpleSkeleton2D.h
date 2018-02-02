#pragma once

#include "Skeleton.h"
#include "SimpleDefinitions.h"

class SimpleSkeleton2D : public Skeleton 
{
private:
	Vector4 m_JointsRest[SMPL_SKELETON_POSITION_COUNT];
	Vector4 m_JointsPose[SMPL_SKELETON_POSITION_COUNT];
	D2D1_POINT_2F m_Points[SMPL_SKELETON_POSITION_COUNT];
	const float m_JointThickness;
	const float m_BoneThickness;

public:
	SimpleSkeleton2D(const float JointThickness = 5.0f, const float BoneThickness = 6.0f);
	void ApplyTransformations(Vector4& scale, Vector4& translate);
	void Render(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pBrushJointSimple,
		ID2D1SolidColorBrush* pBrushBoneSimple, int width, int height);
};