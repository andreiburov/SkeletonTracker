#pragma once

#include <stdio.h>

namespace util {

	Vector4 VECTOR4(float x, float y, float z, float w);
	Vector4 VECTOR4(float x, float y, float z);
	Vector4 VECTOR4(DirectX::XMVECTOR v);
	DirectX::XMVECTOR QUATERNION(Vector4 v);
	double Length(Vector4 v);
	Vector4 Normalize(Vector4 v);
	Vector4 ScalarMultiplication(Vector4 v, double s);
	Vector4 AxisAngleFromQuaternion(Vector4 q);

	struct Vec4 : Vector4 {
		Vec4(const Vector4& v);
		friend std::wostream& operator<<(std::wostream& out, const Vec4& v);
	};
} //util