#pragma once

#include <stdio.h>

#define SAFE_RELEASE(p) do { if (p) { (p)->Release(); (p) = nullptr; } } while(0)
#define VALIDATE(x, wstr) \
do { HRESULT hr = (x); if( FAILED(hr) ) \
{ MessageBoxW(NULL, wstr, L"File error", MB_ICONERROR | MB_OK); } } while(0)
#define V_RETURN(x, wstr) \
do { HRESULT hr = (x); if( FAILED(hr) ) \
{ MessageBoxW(NULL, wstr, L"File error", MB_ICONERROR | MB_OK); return hr; } } while(0)

namespace util {

	Vector4 VECTOR4(float x, float y, float z, float w);
	Vector4 VECTOR4(float x, float y, float z);
	Vector4 VECTOR4(DirectX::XMVECTOR v);
	DirectX::XMVECTOR QUATERNION(Vector4 v);
	float Length(Vector4 v);
	Vector4 Normalize(Vector4 v);
	Vector4 AxisAngleFromQuaternion(Vector4 q);

	std::vector<byte> readShaderFromCSO(const std::wstring& filename);

	struct Vec4 : Vector4 {
		Vec4(const Vector4& v);
		friend std::wostream& operator<<(std::wostream& out, const Vec4& v);
	};

	struct TempFile {
		std::wofstream file;

		TempFile(std::string path);
		~TempFile();
		TempFile& operator<<(const Vector4& v);

		template<typename T>
		TempFile& operator<<(const T& e);
	};

	template<typename T>
	TempFile& TempFile::operator<<(const T& e)
	{ 
		file << e; 
		return *this; 
	};
} //util