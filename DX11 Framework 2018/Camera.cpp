#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowHeight, FLOAT windowWidth, FLOAT nearDepth, FLOAT farDepth, FLOAT angle)
{
	_eye = position;
	_at = at;
	_up = up;

	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	_yAngle = angle;

	// Initialize the view matrix
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(GetPositionXM(), GetLookAtXM(), GetUpXM()) * XMMatrixRotationY(_yAngle));
	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));
}

Camera::~Camera()
{

}

void Camera::Update(float t)
{
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(GetPositionXM(), GetLookAtXM(), GetUpXM()));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));
}

///////////////////////
///////////////////////
///////////////////////

XMFLOAT3 Camera::GetPosition()
{
	return _eye;
}

XMVECTOR Camera::GetPositionXM()
{
	return XMLoadFloat3(&_eye);
}

void Camera::SetPosition(float x, float y, float z)
{
	_eye = { x, y, z };
}

XMFLOAT3 Camera::GetLookAt()
{
	return _at;
}

XMVECTOR Camera::GetLookAtXM()
{
	return XMLoadFloat3(&_at);
}

void Camera::SetLookAt(float x, float y, float z)
{
	_at = { x, y, z };
}

XMFLOAT3 Camera::GetUp()
{
	return _up;
}

XMVECTOR Camera::GetUpXM()
{
	return XMLoadFloat3(&_up);
}

void Camera::SetUp(float x, float y, float z)
{
	_up = { x, y, z };
}

///////////////////////
///////////////////////
///////////////////////

XMFLOAT4X4 Camera::GetViewMx()
{
	return _view;
}

XMFLOAT4X4 Camera::GetProjectionMx()
{
	return _projection;
}

XMFLOAT4X4 Camera::GetViewProjectionMx()
{
	XMFLOAT4X4 viewProjection;
	XMStoreFloat4x4(&viewProjection, XMMatrixMultiply(XMLoadFloat4x4(&GetProjectionMx()), XMLoadFloat4x4(&GetViewMx()))); //view projection

	return viewProjection;
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{

}
