#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

using namespace DirectX;

class Camera
{
private:
	XMFLOAT3 _eye;
	XMFLOAT3 _at;
	XMFLOAT3 _up;

	FLOAT _windowHeight;
	FLOAT _windowWidth;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowHeight, FLOAT windowWidth, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void Update();

	XMFLOAT3 GetPosition();
	XMVECTOR GetPositionXM();
	XMFLOAT3 GetLookAt();
	XMVECTOR GetLookAtXM();
	XMFLOAT3 GetUp();
	XMVECTOR GetUpXM();

	void SetPosition(float x, float y, float z);
	void SetLookAt(float x, float y, float z);
	void SetUp(float x, float y, float z);

	XMFLOAT4X4 GetViewMx();
	XMFLOAT4X4 GetProjectionMx();
	XMFLOAT4X4 GetViewProjectionMx();

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT Depth, FLOAT farDepth);
};