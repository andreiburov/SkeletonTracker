#include "stdafx.h"
#include "Utils.h"

Vector4 VECTOR4(float x, float y, float z, float w)
{
	Vector4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

Vector4 VECTOR4(float x, float y, float z)
{
	return VECTOR4(x, y, z, 1);
}