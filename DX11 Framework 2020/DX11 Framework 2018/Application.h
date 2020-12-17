#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include <vector>
#include "DDSTextureLoader.h"
#include "OBJLoader.h"
#include "Structures.h"
#include "Constants.h"
#include "GameObject.h"
#include "Camera.h"

using namespace DirectX;

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
	ID3D11PixelShader*      _pNoTexPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	//rasterizer
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solidRaster;
	ID3D11RasterizerState* _noCulling;

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
	XMFLOAT4 diffuseMtrl;
	XMFLOAT4 diffuseLight;
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;

	//texture
	ID3D11ShaderResourceView* _pTextureRV;
	ID3D11SamplerState* _pSamplerLinear;

	//Mesh Data
	std::vector<MeshData> _meshData;

	//cameras
	std::vector <Camera*> camera;
	int selectedCamera;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

	bool rasterizerState;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Input();
	void Draw();
};

