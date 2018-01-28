#include "stdafx.h"
#include "SimpleModel.h"
#include "Utils.h"

#define VERTEX_COUNT 6890
#define FACE_COUNT 13776

void SimpleModel::Create(ID3D11Device* pd3dDevice, const std::wstring& modelFilename,
	const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename)
{
	const D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	std::vector<byte> vertexShader = util::readShaderFromCSO(vertexShaderFilename);
	VALIDATE(pd3dDevice->CreateVertexShader(vertexShader.data(), vertexShader.size(),
		nullptr, &m_pVertexShader), L"Could not create VertexShader");

	VALIDATE(pd3dDevice->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc),
		vertexShader.data(), vertexShader.size(), &m_pInputLayout), L"Could not create InputLayout");

	std::vector<byte> pixelShader = util::readShaderFromCSO(pixelShaderFilename);
	VALIDATE(pd3dDevice->CreatePixelShader(pixelShader.data(), pixelShader.size(),
		nullptr, &m_pPixelShader), L"Could not create PixelShader");

	std::vector<SimpleVertex> vertices;
	std::vector<unsigned short> indices;

	readObjFile(modelFilename, vertices, indices);

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	VALIDATE(pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
		&m_pVertexBuffer), L"Could not create VertexBuffer");

	m_IndicesCount = indices.size();

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * indices.size();
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = indices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	VALIDATE(pd3dDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_pIndexBuffer),
		L"Could not create IndexBuffer");

	D3D11_BUFFER_DESC constantBufferDesc = { 0 };
	constantBufferDesc.ByteWidth = sizeof(m_ConstantBufferData);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	
	VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pConstantBuffer),
		L"Could not create ConstantBuffer");

	m_ConstantBufferData.view = DirectX::XMFLOAT4X4(
		-1.00000000f, 0.00000000f, 0.00000000f, 0.00000000f,
		0.00000000f, 0.89442718f, 0.44721359f, 0.00000000f,
		0.00000000f, 0.44721359f, -0.89442718f, -2.23606800f,
		0.00000000f, 0.00000000f, 0.00000000f, 1.00000000f);

	m_ConstantBufferData.projection = DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, -.1f,
		0.0f, 0.0f, -1.0f, 0.0f
	);
}

void SimpleModel::Render(ID3D11DeviceContext* pd3dDeviceContext)
{
	static float degree = 0.f;
	DirectX::XMFLOAT4X4 rotationY;
	DirectX::XMStoreFloat4x4(&rotationY, DirectX::XMMatrixRotationY(degree));

	// Update the constant buffer to rotate the cube model.
	m_ConstantBufferData.model = rotationY;
	degree += 0.01f;

	pd3dDeviceContext->UpdateSubresource(m_pConstantBuffer,	0, nullptr,	&m_ConstantBufferData, 0, 0);

	pd3dDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set the vertex and index buffers, and specify the way they define geometry.
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pd3dDeviceContext->IASetVertexBuffers(0, 1,	&m_pVertexBuffer, &stride, &offset);
	pd3dDeviceContext->IASetIndexBuffer(m_pIndexBuffer,	DXGI_FORMAT_R16_UINT, 0);
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex and pixel shader stage state.
	pd3dDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	pd3dDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pd3dDeviceContext->PSSetShader(m_pPixelShader, nullptr,	0);

	// Draw the cube.
	pd3dDeviceContext->DrawIndexed(m_IndicesCount, 0, 0);
}

void SimpleModel::Clear()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pConstantBuffer);
}

void SimpleModel::readObjFile(const std::wstring& filename,
	std::vector<SimpleVertex> vertices, std::vector<unsigned short>& indices)
{
	std::ifstream file(filename, std::ios::in);
	if (!file)
	{
		MessageBoxW(NULL, (std::wstring(L"Could not open ") + filename).c_str(), L"File error", MB_ICONERROR | MB_OK);
	}

	vertices.reserve(VERTEX_COUNT);

	vertices.insert(vertices.begin(),
		std::istream_iterator<SimpleVertex>(file),
		std::istream_iterator<SimpleVertex>());

	indices.reserve(FACE_COUNT * 3);

	indices.insert(indices.begin(),
		std::istream_iterator<unsigned short>(file),
		std::istream_iterator<unsigned short>());
}