#include "stdafx.h"
#include "SimpleSkeleton3D.h"
#include "Utils.h"

SimpleSkeleton3D::SimpleSkeleton3D()
{
	for (unsigned int i = 0; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		SimpleSkeleton3DVertex v;
		ZeroMemory(&v, sizeof(v));
		v.pos[0] = g_SimpleSkeleton[(_SMPL_SKELETON_POSITION_INDEX)i].x;
		v.pos[1] = g_SimpleSkeleton[(_SMPL_SKELETON_POSITION_INDEX)i].y;
		v.pos[2] = g_SimpleSkeleton[(_SMPL_SKELETON_POSITION_INDEX)i].z;
		m_Vertices[i] = v;
	}
}

void SimpleSkeleton3D::Create(ID3D11Device* pd3dDevice, const std::wstring& vertexShaderFilename,
	const std::wstring& geometryShaderFilename, const std::wstring& pixelShaderFilename,
	float aspectRatio)
{
	const D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "AXIS_ANGLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	std::vector<byte> vertexShader = util::readShaderFromCSO(vertexShaderFilename);
	VALIDATE(pd3dDevice->CreateVertexShader(vertexShader.data(), vertexShader.size(),
		nullptr, &m_pVertexShader), L"Could not create VertexShader");

	VALIDATE(pd3dDevice->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc),
		vertexShader.data(), vertexShader.size(), &m_pInputLayout), L"Could not create InputLayout");

	std::vector<byte> geometryShader = util::readShaderFromCSO(geometryShaderFilename);
	VALIDATE(pd3dDevice->CreateGeometryShader(geometryShader.data(), geometryShader.size(),
		nullptr, &m_pGeometryShader), L"Could not create GeometryShader");

	std::vector<byte> pixelShader = util::readShaderFromCSO(pixelShaderFilename);
	VALIDATE(pd3dDevice->CreatePixelShader(pixelShader.data(), pixelShader.size(),
		nullptr, &m_pPixelShader), L"Could not create PixelShader");

	std::vector<unsigned short> indices;
	indices.reserve((SMPL_SKELETON_POSITION_COUNT - 1) * 2);
	for (unsigned short i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		unsigned short parent = SMPL_PARENT_INDEX[i];
		indices.push_back(parent);
		indices.push_back(i);
	}

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(m_Vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = &m_Vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	VALIDATE(pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
		&m_pVertexBuffer), L"Could not create VertexBuffer");

	m_IndicesCount = (unsigned int)indices.size();

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
		constantBufferDesc.ByteWidth = sizeof(m_WVPMatricesConstantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pWVPMatricesConstantBuffer),
			L"Could not create WVPMatricesConstantBuffer");
	}

	DirectX::XMFLOAT4 eye(0, -0.3, -2, 1);
	DirectX::XMFLOAT4 focus(0, -0.3, 0, 1);
	DirectX::XMFLOAT4 up(0, 1, 0, 0);
	m_View = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat4(&eye),
		DirectX::XMLoadFloat4(&focus),
		DirectX::XMLoadFloat4(&up));

	DirectX::XMStoreFloat4x4(&m_WVPMatricesConstantBufferData.projection,
		DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, 0.001f, 10.0f)));

	// Create hierarchy ConstantBuffer for linear blend skinning
	{
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = m_LBS.getByteWidth();
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		VALIDATE(pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_pLBSConstantBuffer),
			L"Could not create HierarchyConstantBuffer");
	}
}

void SimpleSkeleton3D::UpdateAxisAngles(const SimpleRotations& rotations)
{
	for (unsigned short i = 1; i < SMPL_SKELETON_POSITION_COUNT; i++)
	{
		DirectX::XMFLOAT4 axisAngle = util::AxisAngle4FromQuaternion(rotations[(_SMPL_SKELETON_POSITION_INDEX)i]);
		m_Vertices[i].axisangle[0] = axisAngle.x;
		m_Vertices[i].axisangle[1] = axisAngle.y;
		m_Vertices[i].axisangle[2] = axisAngle.z;
		m_Vertices[i].axisangle[3] = axisAngle.w;
	}
}

void SimpleSkeleton3D::Render(ID3D11DeviceContext* pd3dDeviceContext, 
	const SimpleRotations& rotations, bool online)
{
	static float degree = DirectX::XM_PI;

	// Update the constant buffer to rotate the cube model
	DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(1, 1.5, 1) *
		DirectX::XMMatrixRotationY(degree)*m_View;
	DirectX::XMStoreFloat4x4(&m_WVPMatricesConstantBufferData.worldView, DirectX::XMMatrixTranspose(transform));

	degree += 0.01f;

	pd3dDeviceContext->UpdateSubresource(m_pWVPMatricesConstantBuffer, 0, nullptr, &m_WVPMatricesConstantBufferData, 0, 0);

	if (!online) rotations.printThetas();

	UpdateAxisAngles(rotations);

	m_LBS.UpdateWithEigenSmplStyle(rotations, !online);
	pd3dDeviceContext->UpdateSubresource(m_pLBSConstantBuffer, 0, nullptr, m_LBS.getHierarchyConstantBuffer(), 0, 0);

	// Update per vertex AxisAngles
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		pd3dDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &m_Vertices, sizeof(m_Vertices));
		pd3dDeviceContext->Unmap(m_pVertexBuffer, 0);
	}

	pd3dDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set the vertex and index buffers, and specify the way they define geometry
	UINT stride = sizeof(SimpleSkeleton3DVertex);
	UINT offset = 0;
	pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pd3dDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	pd3dDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	pd3dDeviceContext->VSSetConstantBuffers(0, 1, &m_pLBSConstantBuffer);

	pd3dDeviceContext->GSSetShader(m_pGeometryShader, nullptr, 0);
	pd3dDeviceContext->GSSetConstantBuffers(0, 1, &m_pWVPMatricesConstantBuffer);

	pd3dDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	pd3dDeviceContext->DrawIndexed(m_IndicesCount, 0, 0);
}

void SimpleSkeleton3D::Clear()
{
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pGeometryShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pLBSConstantBuffer);
}