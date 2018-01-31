#include "stdafx.h"
#include "SimpleModel.h"
#include "Utils.h"

#define VERTEX_COUNT 6890
#define FACE_COUNT 13776
#define MAX_TRIANGLES_PER_VERTEX 9
#define SMPL_POSEDIRS_COUNT 207
#define SMPL_POSEDIRS_ELEMENTS_COUNT 4278690

void SimpleModel::Create(ID3D11Device* pd3dDevice, const std::string& modelFilename,
	const std::string& posedirsFilename, const std::wstring& vertexShaderFilename, 
	const std::wstring& pixelShaderFilename, float aspectRatio)
{
	const D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINT_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
	float* posedirs = new float[SMPL_POSEDIRS_ELEMENTS_COUNT];

	readObjFile(modelFilename, posedirsFilename, vertices, indices, posedirs);
	computeFaceNormals(vertices, indices);

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * (unsigned int)vertices.size();
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
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * (unsigned int)indices.size();
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

	// Create vertex ConstantBuffer for world view perspective matrices
	{
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = sizeof(m_VertexConstantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pVertexConstantBuffer),
			L"Could not create VertexConstantBuffer");
	}

	DirectX::XMFLOAT4 eye(0, 1, -2, 1);
	DirectX::XMFLOAT4 focus(0, 0, 0, 1);
	DirectX::XMFLOAT4 up(0, 1, 0, 0);
	m_View = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat4(&eye),
		DirectX::XMLoadFloat4(&focus),
		DirectX::XMLoadFloat4(&up));

	DirectX::XMStoreFloat4x4(&m_VertexConstantBufferData.projection,
		DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, 0.001f, 10.0f)));

	// Create hierarchy ConstantBuffer for linear blend skinning
	{
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = sizeof(m_HierarchyConstantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pHierarchyConstantBuffer),
			L"Could not create HierarchyConstantBuffer");
	}

	// Create theta ConstantBuffer for linear blend skinning
	/*{
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = sizeof(m_HierarchyConstantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pHierarchyConstantBuffer),
			L"Could not create HierarchyConstantBuffer");
	}*/

	{
		D3D11_BUFFER_DESC posedirsBufferDesc = { 0 };
		posedirsBufferDesc.ByteWidth = sizeof(float)* VERTEX_COUNT * SMPL_POSEDIRS_COUNT * 3;
		posedirsBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		posedirsBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		posedirsBufferDesc.CPUAccessFlags = 0;
		posedirsBufferDesc.MiscFlags = 0;
		posedirsBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA posedirsBufferData;
		posedirsBufferData.pSysMem = posedirs;
		posedirsBufferData.SysMemPitch = 0;
		posedirsBufferData.SysMemSlicePitch = 0;

		ID3D11Buffer* pPosedirsConstantBuffer;

		VALIDATE(pd3dDevice->CreateBuffer(&posedirsBufferDesc, &posedirsBufferData, &pPosedirsConstantBuffer),
			L"Could not create posedirs");
		
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = VERTEX_COUNT * SMPL_POSEDIRS_COUNT;

		VALIDATE(pd3dDevice->CreateShaderResourceView(pPosedirsConstantBuffer, &srvDesc, &m_pPosedirsSRV),
			L"Could not create posedirs SRV");
	}

	delete[] posedirs;
}

void SimpleModel::Render(ID3D11DeviceContext* pd3dDeviceContext)
{
	static float degree = 0.f;
	static float elapsed = 0.f;
	static const float factor = 3;
	static const float range = DirectX::XM_PIDIV2/3.f;

	// Update the constant buffer to rotate the cube model
	DirectX::XMMATRIX transform = 
		DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(degree), m_View);
	DirectX::XMStoreFloat4x4(&m_VertexConstantBufferData.worldView, DirectX::XMMatrixTranspose(transform));
	DirectX::XMStoreFloat4x4(&m_VertexConstantBufferData.worldViewIT, 
		DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(transform))));

	degree += 0.01f;
	elapsed += 0.01f;

	pd3dDeviceContext->UpdateSubresource(m_pVertexConstantBuffer, 0, nullptr, &m_VertexConstantBufferData, 0, 0);
	
	float angle = std::abs(std::remainder(elapsed * factor, range * 2));
	SimpleHierarchy::SimpleRotations rotations;
	rotations[SMPL_SKELETON_POSITION_WRIST_LEFT] = 
		DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationZ(angle/10.f));
	rotations[SMPL_SKELETON_POSITION_ELBOW_LEFT] =
		DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationY(angle/5.f));
	rotations[SMPL_SKELETON_POSITION_HIP_RIGHT] = 
		DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationX(-angle));
	rotations[SMPL_SKELETON_POSITION_KNEE_RIGHT] =
		DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationX(angle));

	m_Hierarchy.Update(rotations, m_HierarchyConstantBufferData);
	pd3dDeviceContext->UpdateSubresource(m_pHierarchyConstantBuffer, 0, nullptr, &m_HierarchyConstantBufferData, 0, 0);

	pd3dDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set the vertex and index buffers, and specify the way they define geometry
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pd3dDeviceContext->IASetVertexBuffers(0, 1,	&m_pVertexBuffer, &stride, &offset);
	pd3dDeviceContext->IASetIndexBuffer(m_pIndexBuffer,	DXGI_FORMAT_R16_UINT, 0);
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex and pixel shader stage state
	pd3dDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	pd3dDeviceContext->VSSetConstantBuffers(0, 1, &m_pVertexConstantBuffer);
	pd3dDeviceContext->VSSetConstantBuffers(1, 1, &m_pHierarchyConstantBuffer);
	pd3dDeviceContext->VSSetShaderResources(0, 1, &m_pPosedirsSRV);
	pd3dDeviceContext->PSSetShader(m_pPixelShader, nullptr,	0);

	// Draw the cube
	pd3dDeviceContext->DrawIndexed(m_IndicesCount, 0, 0);
}

void SimpleModel::Clear()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexConstantBuffer);
	SAFE_RELEASE(m_pHierarchyConstantBuffer);
	SAFE_RELEASE(m_pPosedirsSRV);
}

void SimpleModel::readObjFile(const std::string& modelFilename,
	const std::string& posedirsFilename, std::vector<SimpleVertex>& vertices,
	std::vector<unsigned short>& indices, float*& posedirs)
{
	std::ifstream modelFile(modelFilename, std::ios::in);
	if (!modelFile)
	{
		MessageBoxW(NULL, (std::wstring(L"Could not open model")).c_str(), L"File error", MB_ICONERROR | MB_OK);
	}

	vertices.reserve(VERTEX_COUNT);
	indices.reserve(FACE_COUNT * 3);

	int i = 0;

	while (modelFile)
	{
		std::string type;
		modelFile >> type;
		if (type.compare("v") == 0)
		{
			float x, y, z;
			modelFile >> x >> y >> z;
			vertices.push_back(SimpleVertex(x, y, z));
		}
		else if (type.compare("f") == 0)
		{
			unsigned short x, y, z;
			modelFile >> x >> y >> z;
			indices.push_back(x - 1);
			indices.push_back(y - 1);
			indices.push_back(z - 1);
		}
		else if (type.compare("#w") == 0)
		{
			for (int j = 0, k = 0; j < SMPL_SKELETON_POSITION_COUNT; j++)
			{
				float weight;
				modelFile >> weight;
				if (weight > 0.00001f) // small epsilon value
				{
					vertices[i].joint_idx[k] = j;
					vertices[i].weight[k] = weight;
					k++;
				}
			}
			i++; // next vertex
		}
	}

	FILE* posedirsFile = nullptr;
	fopen_s(&posedirsFile, posedirsFilename.c_str(), "rb");
	if (!posedirsFile)
	{
		MessageBoxW(NULL, (std::wstring(L"Could not open posedirs")).c_str(), L"File error", MB_ICONERROR | MB_OK);
	}
	fread(posedirs, sizeof(float), SMPL_POSEDIRS_ELEMENTS_COUNT, posedirsFile);
	fclose(posedirsFile);
}

void SimpleModel::computeFaceNormals(std::vector<SimpleVertex>& vertices, const std::vector<unsigned short>& indices)
{
	unsigned short n_count[VERTEX_COUNT];
	DirectX::XMFLOAT3 normals[VERTEX_COUNT][MAX_TRIANGLES_PER_VERTEX];
	ZeroMemory(n_count, sizeof(n_count));
	ZeroMemory(normals, sizeof(normals));

	for (size_t j = 0; j < indices.size(); j+=3)
	{
		auto i1 = indices[j];
		auto i2 = indices[j + 1];
		auto i3 = indices[j + 2];

		DirectX::XMFLOAT3 v1(vertices[i1].pos);
		DirectX::XMFLOAT3 v2(vertices[i2].pos);
		DirectX::XMFLOAT3 v3(vertices[i3].pos);

		DirectX::XMVECTOR u = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2));
		DirectX::XMVECTOR v = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v3));
		DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

		DirectX::XMStoreFloat3(&normals[i1][n_count[i1]], n);
		DirectX::XMStoreFloat3(&normals[i2][n_count[i2]], n);
		DirectX::XMStoreFloat3(&normals[i3][n_count[i3]], n);

		n_count[i1]++;
		n_count[i2]++;
		n_count[i3]++;
	}

	for (size_t i = 0; i < vertices.size(); i++)
	{
		DirectX::XMVECTOR acc(DirectX::XMVectorZero());
		for (unsigned short j = 0; j < n_count[i]; j++)
		{
			acc = DirectX::XMVectorAdd(acc, DirectX::XMLoadFloat3(&normals[i][j]));
		}
		acc = DirectX::XMVector3Normalize(acc);
		
		DirectX::XMFLOAT3 temp_acc;
		DirectX::XMStoreFloat3(&temp_acc, acc);
		vertices[i].nor[0] = temp_acc.x;
		vertices[i].nor[1] = temp_acc.y;
		vertices[i].nor[2] = temp_acc.z;
	}
}