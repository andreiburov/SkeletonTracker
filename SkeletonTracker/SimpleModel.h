#pragma once

class SimpleModel
{
public:
	struct SimpleVertex
	{
		float pos[3];
		float color[3];
	};

	SimpleModel() {}

	void Create(ID3D11Device* pd3dDevice, const std::wstring& modelFilename,
		const std::wstring& vertexShaderFileName, 
		const std::wstring& pixelShaderFilename);

	void Clear();

private:
	ID3D11InputLayout * m_pInputLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11VertexShader* m_pPixelShader;
};