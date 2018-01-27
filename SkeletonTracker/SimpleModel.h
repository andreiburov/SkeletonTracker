#pragma once

class SimpleModel
{
public:
	struct SimpleVertex
	{
		float pos[3];
		float color[3];

		SimpleVertex(float posx, float posy, float posz,
			float colx, float coly, float colz)
			: pos{ posx,posy,posz }, color{ colx, coly, colz } {}
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
	size_t m_IndicesCount = 0;
};