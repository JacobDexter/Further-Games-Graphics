#pragma once
#include <windows.h>
#include <directxmath.h>
#include <d3d11.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoords;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct ConstantBuffer
{
	//view matrixs
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	//diffuse
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW; //light pos

	//update var
	float t;

	//ambient
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;

	//specular
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;

	//camera
	XMFLOAT3 EyePosW;
};

struct PerObjectCBuffer
{
	XMFLOAT4X4 worldPosition;
	XMMATRIX world;
};