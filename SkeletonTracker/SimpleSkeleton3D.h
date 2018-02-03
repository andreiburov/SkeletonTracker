#pragma once

#include "SimpleDefinitions.h"
#include "SimpleLBS.h"

class SimpleSkeleton3D
{
public:

	SimpleSkeleton3D();
	~SimpleSkeleton3D() { Clear(); }
	void Create(ID3D11Device* pd3dDevice, const std::wstring& vertexShaderFilename,
		const std::wstring& geometryShaderFilename, const std::wstring& pixelShaderFilename, 
		float aspectRatio);
	void Render(ID3D11DeviceContext*, const SimpleRotations& rotations, bool traceable);
	void ApplyTransformations(Vector4 scaleXYZ, Vector4 rotateXY);
	void Clear();

private:

	void UpdateAxisAngles(const SimpleRotations& rotations);

	struct SimpleSkeleton3DVertex
	{
		float axisangle[4];
		float pos[3];
	};

	struct WVPMatricesConstantBuffer {
		DirectX::XMFLOAT4X4 worldView;
		DirectX::XMFLOAT4X4 projection;
	};

	ID3D11VertexShader * m_pVertexShader = nullptr;
	ID3D11GeometryShader* m_pGeometryShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;

	ID3D11InputLayout * m_pInputLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	unsigned int m_IndicesCount = 0;

	// World/View/Perspective Matrices
	ID3D11Buffer* m_pWVPMatricesConstantBuffer = nullptr;
	WVPMatricesConstantBuffer m_WVPMatricesConstantBufferData;
	DirectX::XMMATRIX m_World;
	DirectX::XMMATRIX m_View;

	// Linear Blend Skinning Matrices
	ID3D11Buffer * m_pLBSConstantBuffer = nullptr;
	SimpleLBS m_LBS;

	SimpleSkeleton3DVertex m_Vertices[SMPL_SKELETON_POSITION_COUNT];
};