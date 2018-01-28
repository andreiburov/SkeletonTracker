#pragma once

class SimpleModel
{
public:
	struct SimpleVertex
	{
		float pos[3];

		SimpleVertex(float posx, float posy, float posz)
			: pos{ posx,posy,posz }{}
	};

	struct ConstantBuffer {
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	SimpleModel() {}
	~SimpleModel() { Clear(); }

	void Create(ID3D11Device* pd3dDevice, const std::wstring& modelFilename,
		const std::wstring& vertexShaderFileName, 
		const std::wstring& pixelShaderFilename);

	void Render(ID3D11DeviceContext*);

	void Clear();

private:
	ID3D11InputLayout * m_pInputLayout = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;
	ConstantBuffer m_ConstantBufferData;
	size_t m_IndicesCount = 0;

	void readObjFile(const std::wstring& filename,
		std::vector<SimpleVertex> vertices, std::vector<unsigned short>& indices)
};