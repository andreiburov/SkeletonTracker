#include "stdafx.h"
#include "Skeleton.h"

float mirror(float x, int width)
{
	return width - x;
}

/// <summary>
/// Converts a skeleton point to screen space
/// </summary>
/// <param name="skeletonPoint">skeleton point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F Skeleton::SkeletonToScreen(Vector4 skeletonPoint, int width, int height)
{
	static const int cScreenWidth = 320;
	static const int cScreenHeight = 240;
	LONG x, y;
	USHORT depth;

	// Calculate the skeleton's position on the screen
	// NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
	NuiTransformSkeletonToDepthImage(skeletonPoint, &x, &y, &depth);

	float screenPointX = mirror(static_cast<float>(x * width) / cScreenWidth, width);
	float screenPointY = static_cast<float>(y * height) / cScreenHeight;

	return D2D1::Point2F(screenPointX, screenPointY);
}