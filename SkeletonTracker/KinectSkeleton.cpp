#include <strsafe.h>

#include "stdafx.h"
#include "KinectSkeleton.h"
#include "Utils.h"

const char* KINECT_NAME_FROM_INDEX[NUI_SKELETON_POSITION_COUNT] = {
	"HIP_CENTER",
	"SPINE",
	"SHOULDER_CENTER",
	"HEAD",
	"SHOULDER_LEFT",
	"ELBOW_LEFT",
	"WRIST_LEFT",
	"HAND_LEFT",
	"SHOULDER_RIGHT",
	"ELBOW_RIGHT",
	"WRIST_RIGHT",
	"HAND_RIGHT",
	"HIP_LEFT",
	"KNEE_LEFT",
	"ANKLE_LEFT",
	"FOOT_LEFT",
	"HIP_RIGHT",
	"KNEE_RIGHT",
	"ANKLE_RIGHT",
	"FOOT_RIGHT"
};

KinectRotations::KinectRotations()
	: hierarchy{
		{ -1, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, SMPL_SKELETON_POSITION_STOMACH},
		{ NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, SMPL_SKELETON_POSITION_CHEST },
		{ NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD, SMPL_SKELETON_POSITION_SHOULDER_CENTER },
		{ NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, SMPL_SKELETON_POSITION_PECK_LEFT },
		{ NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, SMPL_SKELETON_POSITION_SHOULDER_LEFT },
		{ NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, SMPL_SKELETON_POSITION_ELBOW_LEFT },
		{ NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT, SMPL_SKELETON_POSITION_WRIST_LEFT },
		{ NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, SMPL_SKELETON_POSITION_PECK_RIGHT },
		{ NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, SMPL_SKELETON_POSITION_SHOULDER_RIGHT },
		{ NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, SMPL_SKELETON_POSITION_ELBOW_RIGHT },
		{ NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT, SMPL_SKELETON_POSITION_WRIST_RIGHT },
		{ NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, SMPL_SKELETON_POSITION_HIP_CENTER },
		{ NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, SMPL_SKELETON_POSITION_HIP_LEFT },
		{ NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, SMPL_SKELETON_POSITION_KNEE_LEFT },
		{ NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT, SMPL_SKELETON_POSITION_ANKLE_LEFT },
		{ NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, SMPL_SKELETON_POSITION_HIP_CENTER },
		{ NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, SMPL_SKELETON_POSITION_HIP_RIGHT },
		{ NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, SMPL_SKELETON_POSITION_KNEE_RIGHT },
		{ NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT, SMPL_SKELETON_POSITION_ANKLE_RIGHT } }
{
	ZeroMemory(rotations, sizeof(rotations));
	for (unsigned short i = 0; i < KINECT_ROTATIONS_COUNT; i++)
	{
		rotations[i] = DirectX::XMQuaternionIdentity();
	}
}

void KinectRotations::printQuaternions() const
{
	util::TempFile file("Rotations/");

	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		DirectX::XMFLOAT4 q;
		XMStoreFloat4(&q, rotations[i]);
		file << "SMPL: " << SMPL_NAME_FROM_INDEX[hierarchy[i].smpl]
			<< " JOINT: " << KINECT_NAME_FROM_INDEX[hierarchy[i].joint]
			<< " CHILD: " << KINECT_NAME_FROM_INDEX[hierarchy[i].child] << "\n";
		file << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "\n";
	}
}

static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

KinectSkeleton::KinectSkeleton() : m_isTposeCalibrated(false)
{
	ZeroMemory(m_Points, sizeof(m_Points));
}

KinectSkeleton::~KinectSkeleton()
{

}

void KinectSkeleton::FixTpose()
{
	memcpy(&m_tPoseOrientations, &m_Orientations, sizeof(m_Orientations));
	m_isTposeCalibrated = true;
}


void KinectSkeleton::GetSimplePose(SimpleRotations& rotations)
{
	for (int i = 0; i < KINECT_ROTATIONS_COUNT; i++)
	{
		rotations[(SMPL_SKELETON_POSITION_INDEX)m_Orientations.hierarchy[i].smpl] =
			util::RotationBetweenQuaternions(m_tPoseOrientations.rotations[i],
				m_Orientations.rotations[i]);
	}
}

void KinectSkeleton::CalculateOrientations()
{
	m_Orientations.rotations[0] = DirectX::XMQuaternionIdentity();
	for (int i = 1; i < KINECT_ROTATIONS_COUNT; i++)
	{
		m_Orientations.rotations[i] = util::ThreePositionsToQuaternion(
			m_SkeletonData.SkeletonPositions[m_Orientations.hierarchy[i].child],
			m_SkeletonData.SkeletonPositions[m_Orientations.hierarchy[i].joint],
			m_SkeletonData.SkeletonPositions[m_Orientations.hierarchy[i].parent]
		);
	}
	m_Orientations.rotations[11] = DirectX::XMQuaternionIdentity();
	m_Orientations.rotations[15] = DirectX::XMQuaternionIdentity();
}

void KinectSkeleton::Update(NUI_SKELETON_FRAME& frame, bool traceable)
{
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = frame.SkeletonData[i].eTrackingState;

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			// Storing positions of some skeleton for possible printing
			auto& skel = frame.SkeletonData[i];
			memcpy(&m_SkeletonData, &skel, sizeof(NUI_SKELETON_DATA));

			// Orientations
			//NuiSkeletonCalculateBoneOrientations(&skel, m_SkeletonOrientations);
			CalculateOrientations();

			util::TempFile file("Joints/");
			for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
			{
				file << KINECT_NAME_FROM_INDEX[i] << "\n";
				file << util::Vec4(m_SkeletonData.SkeletonPositions[i]) << "\n";
			}
			file << "\n";
			
			m_Orientations.printQuaternions();
		}
	}
}

void KinectSkeleton::Render(const RenderHelper& helper)
{
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(m_SkeletonData.SkeletonPositions[i], helper.windowWidth, helper.windowHeight);
	}

	// Render Torso
	DrawBone(helper, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	DrawBone(helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	DrawBone(helper, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	DrawBone(helper, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	DrawBone(helper, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// Left Arm
	DrawBone(helper, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// Right Arm
	DrawBone(helper, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	DrawBone(helper, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	DrawBone(helper, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// Left Leg
	DrawBone(helper, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	DrawBone(helper, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// Right Leg
	DrawBone(helper, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	DrawBone(helper, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	DrawBone(helper, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	// Draw the joints in a different color
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(m_Points[i], g_JointThickness, g_JointThickness);

		if (m_SkeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED)
		{
			helper.pRenderTarget->DrawEllipse(ellipse, helper.pBrushJointInferred);
		}
		else if (m_SkeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED)
		{
			helper.pRenderTarget->DrawEllipse(ellipse, helper.pBrushJointTracked);
		}
	}
}

void KinectSkeleton::DrawBone(const RenderHelper& helper,
	NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
	NUI_SKELETON_POSITION_TRACKING_STATE joint0State = m_SkeletonData.eSkeletonPositionTrackingState[joint0];
	NUI_SKELETON_POSITION_TRACKING_STATE joint1State = m_SkeletonData.eSkeletonPositionTrackingState[joint1];

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