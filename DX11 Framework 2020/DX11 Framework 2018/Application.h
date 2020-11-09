#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include <vector>
using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};

struct ConstantBuffer
{
	//update var
	float t;

	//view matrixs
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	//diffuse
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW; //light pos

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

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;

	//shader/rendering variables
	IDXGISwapChain* _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	//rasterizer
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solidRaster;

	//view matrixs
	XMFLOAT4X4 _world, _world2;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	//buffers
	ID3D11Buffer* _VertexBuffers[2];
	ID3D11Buffer* _pyramidIndexBuffer;
	ID3D11Buffer* _cubeIndexBuffer;
	ID3D11Buffer* _pConstantBuffer;
	ConstantBuffer cb;

	//lighting
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	//HRESULT CalculateNormals(int triCount, SimpleVertex vertexBuffer[], WORD indexBuffer[]);

	UINT _WindowHeight;
	UINT _WindowWidth;

	bool rasterizerState;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

