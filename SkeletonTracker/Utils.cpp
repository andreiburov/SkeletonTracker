#include "stdafx.h"
#include "Utils.h"

namespace util {

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

	Vector4 VECTOR4(DirectX::XMVECTOR v)
	{
		return VECTOR4(DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), 
			DirectX::XMVectorGetZ(v), DirectX::XMVectorGetW(v));
	}

	DirectX::XMVECTOR QUATERNION(Vector4 v)
	{
		//return DirectX::XMVectorSet(-v.x, -v.y, v.z, v.w);
		return DirectX::XMVectorSet(-v.x, -v.y, v.z, v.w);
	}

	double Length(Vector4 v)
	{
		return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	Vector4 Normalize(Vector4 v)
	{
		double length = Length(v);
		return VECTOR4(v.x / length, v.y / length, v.z / length);
	}

	Vector4 ScalarMultiplication(Vector4 v, double s)
	{
		return VECTOR4(v.x * s, v.y * s, v.z * s, 1);
	}

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/index.htm

	Vector4 AxisAngleFromQuaternion(Vector4 q)
	{
		Vector4 v;

		if (q.w > 1) q = Normalize(q);
		float angle = 2 * acos(q.w);
		double s = sqrt(1 - q.w*q.w);

		if (s < 0.001)
			v = VECTOR4(q.x, q.y, q.z);
		else
			v = VECTOR4(q.x / s, q.y / s, q.z / s);

		v = Normalize(v);
		v = ScalarMultiplication(v, angle);
		return v;
	}

	std::wostream& operator<<(std::wostream& out, const Vec4& v)
	{
		out << v.x << L" " << v.y << L" " << v.z << L" " << v.z;
		return out;
	}

	Vec4::Vec4(const Vector4& v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
	}

} // util