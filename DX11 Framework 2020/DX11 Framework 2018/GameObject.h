#pragma once
#include "Structures.h"

class GameObject
{
public:
	//constructor/deconstructor
	GameObject(MeshData* mesh, ID3D11DeviceContext* pContext, ID3D11Buffer* cBuffer, ConstantBuffer* constantBuff);
	~GameObject();

	void Draw();
	void Update(float t);

public:
	MeshData* _mesh;
	ID3D11DeviceContext* _pImmediateContext;
	ID3D11Buffer* _pConstantBuffer;
	ConstantBuffer* _cb;
	XMFLOAT4X4 worldPosition;
};