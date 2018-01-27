//------------------------------------------------------------------------------
// <copyright file="SkeletonTracker.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include <strsafe.h>

#include "stdafx.h"
#include "KinectSkeleton.h"
#include "Utils.h"

static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

KinectSkeleton::KinectSkeleton()
{
	ZeroMemory(m_Points, sizeof(m_Points));
}

KinectSkeleton::~KinectSkeleton()
{

}

void KinectSkeleton::PrintJoints()
{
	util::TempFile file("Joints");

	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		file << m_SkeletonPositions[i] << L"\n";
	}
}

void KinectSkeleton::FixTpose()
{
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		m_SkeletonTposeOrientations[i] = m_SkeletonOrientations[i];
	}
}

void KinectSkeleton::PrintSimplePose()
{
	Vector4 rotations[SMPL_SKELETON_POSITION_COUNT];
	ZeroMemory(&rotations, SMPL_SKELETON_POSITION_COUNT * sizeof(Vector4));

	util::TempFile file("Rotations");

#ifdef _DEBUG
	file << L"Index\n";
	file << L"T-Pose Quaternion, T-Pose angle, T-Pose Axis 3Normalized, T-Pose AxisAngle\n";
	file << L"Posed Quaternion, Posed angle, Posed Axis 3Normalized, Posed AxisAngle\n";
	file << L"Inversed T-Pose Quaternion, Multiplied Posed*T-Pose inverse\n";
	file << L"Angle, 3Normalized Axis, AxisAngle\n";
#endif

	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		DirectX::XMVECTOR q = util::QUATERNION(m_SkeletonOrientations[i].hierarchicalRotation.rotationQuaternion);
		DirectX::XMVECTOR tpose_q = util::QUATERNION(m_SkeletonTposeOrientations[i].hierarchicalRotation.rotationQuaternion);
		float angle;
		DirectX::XMVECTOR axis;
		DirectX::XMVECTOR inv_t = DirectX::XMQuaternionInverse(tpose_q);
		DirectX::XMVECTOR mult = DirectX::XMQuaternionMultiply(q, inv_t);
		DirectX::XMQuaternionToAxisAngle(&axis, &angle, mult);
		axis = DirectX::XMVector3Normalize(axis);
		if (angle < 0) angle += DirectX::XM_2PI;
		DirectX::XMVECTOR axis_angle = DirectX::XMVectorScale(axis, angle);
		rotations[SMPL_INDEX_FROM_KINECT_INDEX[m_SkeletonOrientations[i].startJoint]] = util::VECTOR4(axis_angle);
#ifdef _DEBUG
		Vector4 ax = util::VECTOR4(axis);
		Vector4 aa = util::VECTOR4(axis_angle);
		Vector4 pq = util::VECTOR4(q);
		Vector4 tq = util::VECTOR4(tpose_q);

		float pa, ta;
		DirectX::XMVECTOR pax, tax;
		DirectX::XMQuaternionToAxisAngle(&pax, &pa, q);
		DirectX::XMQuaternionToAxisAngle(&tax, &ta, tpose_q);

		Vector4 paxv = util::VECTOR4(DirectX::XMVector3Normalize(pax));
		Vector4 taxv = util::VECTOR4(DirectX::XMVector3Normalize(tax));

		Vector4 paav = util::VECTOR4(DirectX::XMVectorScale(DirectX::XMVector3Normalize(pax), pa));
		Vector4 taav = util::VECTOR4(DirectX::XMVectorScale(DirectX::XMVector3Normalize(tax), ta));
		file << i << L"\n";
		file << tq << L", " << ta << L", " << taxv << L", " << taav << L"\n";
		file << pq << L", " << pa << L", " << paxv << L", " << paav << L"\n";
		file << util::VECTOR4(inv_t) << L", " << util::VECTOR4(mult) << L"\n";
		file << angle << L", " << ax << L", " << aa << L"\n";
		file << L"------------------------------------------------------------" << L"\n";
#endif
	}

	file << L"[ ";
	int n = SMPL_SKELETON_POSITION_COUNT - 1;
	for (int i = 0; i < n; i++)
	{
		file << rotations[i].x << L", " << rotations[i].y*-1. << L", " << rotations[i].z*-1. << L", ";
	}
	file << rotations[n].x << L", " << rotations[n].y*-1. << L", " << rotations[n].z*-1. << L" ]";
}

void KinectSkeleton::Render(const RenderHelper& helper)
{
	NUI_SKELETON_FRAME& frame = *helper.pSkeletonFrame;
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = frame.SkeletonData[i].eTrackingState;

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			// Storing positions of some skeleton for possible printing
			auto& skel = frame.SkeletonData[i];

			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
				m_SkeletonPositions[j] = skel.SkeletonPositions[j];

			// We're tracking the skeleton, draw it
			DrawSkeleton(frame.SkeletonData[i], helper);
		}
		else if (NUI_SKELETON_POSITION_ONLY == trackingState)
		{
			// we've only received the center point of the skeleton, draw that
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				SkeletonToScreen(frame.SkeletonData[i].Position, helper.windowWidth, helper.windowHeight),
				g_JointThickness,
				g_JointThickness
			);

			helper.pRenderTarget->DrawEllipse(ellipse, helper.pBrushJointTracked);
		}
	}
}

void KinectSkeleton::DrawSkeleton(const NUI_SKELETON_DATA & skel, const RenderHelper& helper)
{
	// Orientations
	NuiSkeletonCalculateBoneOrientations(&skel, m_SkeletonOrientations);

	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(skel.SkeletonPositions[i], helper.windowWidth, helper.windowHeight);
	}

	// Render Torso
	DrawBone(skel, helper, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// Left Arm
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// Right Arm
	DrawBone(skel, helper, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// Left Leg
	DrawBone(skel, helper, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// Right Leg
	DrawBone(skel, helper, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	DrawBone(skel, helper, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	// Draw the joints in a different color
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(m_Points[i], g_JointThickness, g_JointThickness);

		if (skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED)
		{
			helper.pRenderTarget->DrawEllipse(ellipse, helper.pBrushJointInferred);
		}
		else if (skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED)
		{
			helper.pRenderTarget->DrawEllipse(ellipse, helper.pBrushJointTracked);
		}
	}
}

void KinectSkeleton::DrawBone(const NUI_SKELETON_DATA & skel, const RenderHelper& helper,
	NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
	NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skel.eSkeletonPositionTrackingState[joint0];
	NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skel.eSkeletonPositionTrackingState[joint1];

	// If we can't find either of these joints, exit
	if (joint0State == NUI_SKELETON_POSITION_NOT_TRACKED || joint1State == NUI_SKELETON_POSITION_NOT_TRACKED)
	{
		return;
	}

	// Don't draw if both points are inferred
	if (joint0State == NUI_SKELETON_POSITION_INFERRED && joint1State == NUI_SKELETON_POSITION_INFERRED)
	{
		return;
	}

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if (joint0State == NUI_SKELETON_POSITION_TRACKED && joint1State == NUI_SKELETON_POSITION_TRACKED)
	{
		helper.pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], helper.pBrushBoneTracked, g_TrackedBoneThickness);
	}
	else
	{
		helper.pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], helper.pBrushBoneInferred, g_InferredBoneThickness);
	}
}