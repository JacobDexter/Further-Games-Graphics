#include "GameObject.h"

GameObject::GameObject(MeshData* mesh, ID3D11DeviceContext* pContext, ID3D11Buffer* constBuffer, ConstantBuffer* cb)
{
	_mesh = mesh;
	_pImmediateContext = pContext;
	_pConstantBuffer = constBuffer;
	_cb = cb;
}

GameObject::~GameObject()
{
	delete _mesh;
}

void GameObject::Draw()
{
	XMMATRIX world = XMLoadFloat4x4(&worldPosition);
	_cb->mWorld = XMMatrixTranspose(world);

	_pImmediateContext->IASetVertexBuffers(0, 1, &_mesh->VertexBuffer, &_mesh->VBStride, &_mesh->VBOffset);
	_pImmediateContext->IASetIndexBuffer(_mesh->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &_cb, 0, 0);

	_pImmediateContext->DrawIndexed(_mesh->IndexCount, 0, 0);
}

void GameObject::Update(float t)
{
	//spin
	XMStoreFloat4x4(&worldPosition, XMMatrixRotationY(t * 0.5f) * XMMatrixTranslation(0.0f, 0.0f, 1.0f));
}
