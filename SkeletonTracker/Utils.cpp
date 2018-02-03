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
		return DirectX::XMVectorSet(-v.x, -v.y, -v.z, v.w);
	}

	Eigen::Quaterniond QUATERNION(float x, float y, float z, float w)
	{
		Eigen::Quaterniond q(w, x, y, z);
		return q;
	}

	DirectX::XMFLOAT4X4 MATRIX(Eigen::Matrix4d in)
	{
		DirectX::XMFLOAT4X4 out;
		out._11 = in(0, 0);
		out._12 = in(0, 1);
		out._13 = in(0, 2);
		out._14 = in(0, 3);
		//
		out._21 = in(1, 0);
		out._22 = in(1, 1);
		out._23 = in(1, 2);
		out._24 = in(1, 3);
		//
		out._31 = in(2, 0);
		out._32 = in(2, 1);
		out._33 = in(2, 2);
		out._34 = in(2, 3);
		//
		out._41 = in(3, 0);
		out._42 = in(3, 1);
		out._43 = in(3, 2);
		out._44 = in(3, 3);

		return out;
	}

	Eigen::Matrix4d RotateAroundPoint(DirectX::XMVECTOR quaternion, DirectX::XMVECTOR point)
	{
		Eigen::Matrix4d rotation = Eigen::Matrix4d::Identity();
		Eigen::Matrix4d translation = Eigen::Matrix4d::Identity();
		Eigen::Matrix4d iTranslation = Eigen::Matrix4d::Identity();
		
		translation.rightCols<1>() = Eigen::Vector4d(DirectX::XMVectorGetX(point),
			DirectX::XMVectorGetY(point), DirectX::XMVectorGetZ(point), DirectX::XMVectorGetW(point));
		iTranslation.rightCols<1>() = Eigen::Vector4d(-DirectX::XMVectorGetX(point),
			-DirectX::XMVectorGetY(point), -DirectX::XMVectorGetZ(point), DirectX::XMVectorGetW(point));
		rotation.block<3, 3>(0, 0) = QUATERNION(DirectX::XMVectorGetX(quaternion), DirectX::XMVectorGetY(quaternion),
			DirectX::XMVectorGetZ(quaternion), DirectX::XMVectorGetW(quaternion)).normalized().toRotationMatrix();
		
		return iTranslation * rotation * translation;
	}

	float Length(Vector4 v)
	{
		return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	Vector4 Normalize(Vector4 v)
	{
		float length = Length(v);
		return VECTOR4(v.x / length, v.y / length, v.z / length);
	}

	Vector4 ScalarMultiplication(Vector4 v, float s)
	{
		return VECTOR4(v.x * s, v.y * s, v.z * s, 1);
	}

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/index.htm

	Vector4 AxisAngleFromQuaternion(Vector4 q)
	{
		Vector4 v;

		if (q.w > 1) q = Normalize(q);
		float angle = 2 * acos(q.w);
		float s = sqrt(1 - q.w*q.w);

		if (s < 0.001)
			v = VECTOR4(q.x, q.y, q.z);
		else
			v = VECTOR4(q.x / s, q.y / s, q.z / s);

		v = Normalize(v);
		v = ScalarMultiplication(v, angle);
		return v;
	}

	DirectX::XMVECTOR AxisAngle3FromQuaternion(const DirectX::XMVECTOR quaternion)
	{
		DirectX::XMFLOAT3 output;
		DirectX::XMVECTOR axis;
		float angle;

		DirectX::XMQuaternionToAxisAngle(&axis, &angle, quaternion);
		axis = DirectX::XMVector3Normalize(axis);
		while (angle < 0) angle += DirectX::XM_2PI;
		while (angle > DirectX::XM_2PI) angle -= DirectX::XM_2PI;
		return DirectX::XMVectorScale(axis, angle);
	}

	DirectX::XMFLOAT4 AxisAngle4FromQuaternion(const DirectX::XMVECTOR quaternion)
	{
		DirectX::XMFLOAT4 out;
		DirectX::XMVECTOR axis;
		float angle;

		DirectX::XMQuaternionToAxisAngle(&axis, &angle, quaternion);
		axis = DirectX::XMVector3Normalize(axis);
		while (angle < 0) angle += DirectX::XM_2PI;
		while (angle > DirectX::XM_2PI) angle -= DirectX::XM_2PI;
		DirectX::XMStoreFloat4(&out, DirectX::XMVectorSetW(axis, angle));
		return out;
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

	std::string getCurrentTime()
	{
		auto t = std::time(nullptr);
		struct tm tm;
		localtime_s(&tm, &t);
		std::ostringstream oss;
		oss << std::put_time(&tm, std::string("%d-%m-%Y_%H-%M-%S").c_str());
		return oss.str();
	}

	TempFile::TempFile(std::string path)
	{
		file.open(path.append(getCurrentTime()).append(".txt"), std::ios::out);
	}

	TempFile::~TempFile()
	{
		file.close();
	}

	TempFile& TempFile::operator<<(const Vector4& v)
	{
		Vec4 vv(v);
		file << vv;
		return *this;
	}

	std::vector<byte> readShaderFromCSO(const std::wstring& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if (!file)
		{
			MessageBoxW(NULL, (std::wstring(L"Could not open ") + filename).c_str(), L"File error", MB_ICONERROR | MB_OK);
		}

		// Stop eating new lines in binary mode!!!
		file.unsetf(std::ios::skipws);

		// get its size:
		std::streampos filesize;

		file.seekg(0, std::ios::end);
		filesize = file.tellg();
		file.seekg(0, std::ios::beg);

		// reserve capacity
		std::vector<byte> v;
		v.reserve((unsigned int)filesize);

		// read the data:
		v.insert(v.begin(),
			std::istream_iterator<byte>(file),
			std::istream_iterator<byte>());

		return v;
	}

} // util