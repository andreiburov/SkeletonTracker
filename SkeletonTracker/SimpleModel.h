#pragma once

#include "SimpleHierarchy.h"
#include "SimplePose.h"

class SimpleModel
{
public:
	struct SimpleVertex
	{
		float weight[4];
		int joint_idx[4];
		float pos[3];
		float nor[3];

		SimpleVertex(float posx, float posy, float posz)
			: pos{ posx,posy,posz }, nor{ 0.f,0.f,0.f } {}
	};

	struct VertexConstantBuffer {
		DirectX::XMFLOAT4X4 worldView;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4X4 worldViewIT;
	};

	SimpleModel() {}
	~SimpleModel() { Clear(); }

	void Create(ID3D11Device* pd3dDevice, const std::string& modelFilename,
		const std::string& posedirsFilename, const std::wstring& vertexShaderFileName, 
		const std::wstring& pixelShaderFilename, float aspectRatio);

	void Render(ID3D11DeviceContext*);

	void Clear();

private:
	ID3D11InputLayout * m_pInputLayout = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	unsigned int m_IndicesCount = 0;
	
	// World/View/Perspective Matrices
	ID3D11Buffer* m_pVertexConstantBuffer = nullptr; 
	VertexConstantBuffer m_VertexConstantBufferData;
	DirectX::XMMATRIX m_View;

	// Hierarchy for Linear Blend Skinning
	ID3D11Buffer* m_pHierarchyConstantBuffer = nullptr;
	SimpleHierarchy m_Hierarchy;

	// Thetas for SMPL
	ID3D11Buffer* m_pPoseConstantBuffer = nullptr;
	SimplePose m_Pose;

	// Per-vertex basis of position directions for SMPL
	ID3D11ShaderResourceView*  m_pPosedirsSRV = nullptr;

	void readObjFile(const std::string& modelFilename, const std::string& posedirsFilename,
		std::vector<SimpleVertex>& vertices, std::vector<unsigned short>& indices,
		float*& posedirs);

	void computeFaceNormals(std::vector<SimpleVertex>& vertices, const std::vector<unsigned short>& indices);
};