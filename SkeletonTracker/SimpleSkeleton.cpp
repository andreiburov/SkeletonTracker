#include "stdafx.h"
#include "SimpleSkeleton.h"
#include "Utils.h"

SimpleSkeleton::SimpleSkeleton(const float jointThickness, const float boneThickness)
	: m_JointThickness(jointThickness), m_BoneThickness(boneThickness)
{
	ZeroMemory(&m_JointsRest, sizeof(Vector4)*SMPL_SKELETON_POSITION_COUNT);
	ZeroMemory(&m_JointsPose, sizeof(Vector4)*SMPL_SKELETON_POSITION_COUNT);

	std::ifstream in("Joints/SMPL_Rest.obj", std::ios::in);
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		std::string vertex;
		Vector4 v;
		v.w = 1.f;
		in >> vertex >> v.x >> v.y >> v.z;
		m_JointsRest[i] = v;
		m_JointsPose[i] = v;
	}
	in.close();

	Vector4 scale = VECTOR4(5.5f, 5.3f, 5.0f);
	Vector4 translate = VECTOR4(0.0f, 2.0f, 12.0f);
	ApplyTransformations(scale, translate);
}

void SimpleSkeleton::ApplyTransformations(Vector4& scale, Vector4& translate)
{
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		m_JointsPose[i].x = m_JointsRest[i].x * scale.x + translate.x;
		m_JointsPose[i].y = m_JointsRest[i].y * scale.y + translate.y;
		m_JointsPose[i].z = m_JointsRest[i].z * scale.z + translate.z;
	}
}

void SimpleSkeleton::Render(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pBrushJointSimple,
	ID2D1SolidColorBrush* pBrushBoneSimple, int windowWidth, int windowHeight)
{
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(m_JointsPose[i], windowWidth, windowHeight);
	}

	// Render Torso
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HIP_CENTER], 
		m_Points[SMPL_SKELETON_POSITION_STOMACH], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_STOMACH],
		m_Points[SMPL_SKELETON_POSITION_BACKBONE], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_BACKBONE],
		m_Points[SMPL_SKELETON_POSITION_SHOULDER_CENTER], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_SHOULDER_CENTER],
		m_Points[SMPL_SKELETON_POSITION_CHIN], pBrushBoneSimple, m_BoneThickness);

	// Render Right Arm
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_CHEST],
		m_Points[SMPL_SKELETON_POSITION_PECK_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_PECK_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_SHOULDER_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_SHOULDER_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_ELBOW_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_ELBOW_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_HAND_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HAND_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_WRIST_RIGHT], pBrushBoneSimple, m_BoneThickness);

	// Render Left Arm
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_CHEST],
		m_Points[SMPL_SKELETON_POSITION_PECK_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_PECK_LEFT],
		m_Points[SMPL_SKELETON_POSITION_SHOULDER_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_SHOULDER_LEFT],
		m_Points[SMPL_SKELETON_POSITION_ELBOW_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_ELBOW_LEFT],
		m_Points[SMPL_SKELETON_POSITION_HAND_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HAND_LEFT],
		m_Points[SMPL_SKELETON_POSITION_WRIST_LEFT], pBrushBoneSimple, m_BoneThickness);

	// Render Right Leg
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HIP_CENTER],
		m_Points[SMPL_SKELETON_POSITION_HIP_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HIP_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_KNEE_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_KNEE_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_ANKLE_RIGHT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_ANKLE_RIGHT],
		m_Points[SMPL_SKELETON_POSITION_FOOT_RIGHT], pBrushBoneSimple, m_BoneThickness);

	// Render Left Leg
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HIP_CENTER],
		m_Points[SMPL_SKELETON_POSITION_HIP_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_HIP_LEFT],
		m_Points[SMPL_SKELETON_POSITION_KNEE_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_KNEE_LEFT],
		m_Points[SMPL_SKELETON_POSITION_ANKLE_LEFT], pBrushBoneSimple, m_BoneThickness);
	pRenderTarget->DrawLine(m_Points[SMPL_SKELETON_POSITION_ANKLE_LEFT],
		m_Points[SMPL_SKELETON_POSITION_FOOT_LEFT], pBrushBoneSimple, m_BoneThickness);
	
	for (int i = 0; i < SMPL_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(m_Points[i], m_JointThickness, m_JointThickness);
		pRenderTarget->DrawEllipse(ellipse, pBrushJointSimple);
	}
}