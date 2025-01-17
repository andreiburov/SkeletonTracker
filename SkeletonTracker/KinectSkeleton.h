#pragma once

#include "Skeleton.h"
#include "SimpleDefinitions.h"
#include "KinectDefinitions.h"

class KinectSkeleton : public Skeleton
{
	static const int        cStatusMessageMaxLen = MAX_PATH * 2;

public:
	struct RenderHelper
	{
		ID2D1HwndRenderTarget*  pRenderTarget;
		ID2D1SolidColorBrush*   pBrushJointTracked;
		ID2D1SolidColorBrush*   pBrushJointInferred;
		ID2D1SolidColorBrush*   pBrushBoneTracked;
		ID2D1SolidColorBrush*   pBrushBoneInferred;
		int windowWidth;
		int windowHeight;
	};

	KinectSkeleton();
	~KinectSkeleton();

	void Update(NUI_SKELETON_FRAME& frame, bool traceable);
	void Render(const RenderHelper& helper);
	void FixTpose();
	void GetSimplePose(SimpleRotations& rotations);
	bool isTposeCalibrated() { return m_isTposeCalibrated; }

private:

	// For printing, only one skeleton supported
	NUI_SKELETON_DATA        m_SkeletonData;
	D2D1_POINT_2F            m_Points[NUI_SKELETON_POSITION_COUNT];
	KinectRotations			 m_tPoseOrientations;
	KinectRotations          m_Orientations;
	bool					 m_isTposeCalibrated;

	void CalculateOrientations();
	void DrawBone(const RenderHelper& helper, 
		NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);
};
