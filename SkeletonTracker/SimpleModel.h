#pragma once

#include "SimpleLBS.h"
#include "SimplePose.h"

class SimpleModel
{
public:

	SimpleModel() {}
	~SimpleModel() { Clear(); }

	void Create(ID3D11Device* pd3dDevice, const std::string& modelFilename,
		const std::string& posedirsFilename, const std::wstring& vertexShaderFileName,
		const std::wstring& geometryShaderFilename,	const std::wstring& pixelShaderFilename, float aspectRatio);

	void Render(ID3D11DeviceContext*, const SimpleRotations& rotations, bool online);
	void ToggleLBSonly() { m_VSParametersConstantBufferData.lbsOnly = !m_VSParametersConstantBufferData.lbsOnly; }

	void Clear();

private:

	struct SimpleModelVertex
	{
		float weight[4];
		int joint_idx[4];
		float pos[3];
		float nor[3];

		SimpleModelVertex(float posx, float posy, float posz)
			: pos{ posx,posy,posz }, nor{ 0.f,0.f,0.f } {}
	};

	struct WVPMatricesConstantBuffer {
		DirectX::XMFLOAT4X4 worldView;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4X4 worldViewIT;
	};

	struct VSParametersConstantBuffer {
		bool lbsOnly;
		byte padding[15];

		VSParametersConstantBuffer() : lbsOnly(false)
		{
			ZeroMemory(&padding, sizeof(padding));
		}
	};

	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11GeometryShader* m_pGeometryShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;

	ID3D11InputLayout * m_pInputLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	unsigned int m_IndicesCount = 0;

	ID3D11Buffer* m_pVSParametersConstantBuffer = nullptr;
	VSParametersConstantBuffer m_VSParametersConstantBufferData;
	
	// World/View/Perspective Matrices
	ID3D11Buffer* m_pWVPMatricesConstantBuffer = nullptr; 
	WVPMatricesConstantBuffer m_WVPMatricesConstantBufferData;
	DirectX::XMMATRIX m_View;

	// Linear Blend Skinning Matrices
	ID3D11Buffer* m_pLBSConstantBuffer = nullptr;
	SimpleLBS m_LBS;

	// Thetas for SMPL
	ID3D11Buffer* m_pPoseConstantBuffer = nullptr;
	SimplePose m_Pose;

	// Per-vertex basis of position directions for SMPL
	ID3D11ShaderResourceView*  m_pPosedirsSRV = nullptr;

	void readObjFile(const std::string& modelFilename, const std::string& posedirsFilename,
		std::vector<SimpleModelVertex>& vertices, std::vector<unsigned short>& indices,
		float*& posedirs);

	void computeFaceNormals(std::vector<SimpleModelVertex>& vertices, const std::vector<unsigned short>& indices);
};