#pragma once

class Skeleton
{
public:
	/// <summary>
	/// Converts a skeleton point to screen space
	/// </summary>
	/// <param name="skeletonPoint">skeleton point to tranform</param>
	/// <param name="width">width (in pixels) of output buffer</param>
	/// <param name="height">height (in pixels) of output buffer</param>
	/// <returns>point in screen-space</returns>
	D2D1_POINT_2F SkeletonToScreen(Vector4 skeletonPoint, int width, int height);
};