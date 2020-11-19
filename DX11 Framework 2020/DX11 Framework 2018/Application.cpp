#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pConstantBuffer = nullptr;
    _pyramidIndexBuffer = nullptr;
    _cubeIndexBuffer = nullptr;

    //texture
    _pTextureRV = nullptr;
    _pSamplerLinear = nullptr;

    //lighting
    lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
    diffuseMtrl = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    AmbientMtrl = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
    AmbientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
    SpecularMtrl = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    SpecularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    SpecularPower = 10.0f;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
    
    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

    //Initialise Mesh Data
    carMeshData = OBJLoader::Load("objects/car.obj", _pd3dDevice);

    //Initialise Objects
    car = new GameObject(&carMeshData, _pImmediateContext, _pConstantBuffer, &cb);

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }
    
	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);

	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &_pVertexLayout);

	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

    //create texture
    hr = CreateDDSTextureFromFile(_pd3dDevice, L"Textures/Crate_COLOR.dds", nullptr, &_pTextureRV);

    if (FAILED(hr))
        return hr;

    //load texture into shader resource
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);

    //defining the texture sampler state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    if (FAILED(hr))
        return hr;

    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear); //set sampler

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex cubeVertices[] =
    {
        //front face
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) }, //bottom front left 
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }, //bottom front right 
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //top front right 
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //top front left 

        //right face
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) }, //bottom front right
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //bottom back right
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }, //top back right
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //top front right

        //top face
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) }, //top front left
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }, //top front right
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }, //top back right
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }, //top back left

        //left face
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //bottom back left
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }, //bottom front left
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //top front left
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }, //top back left

        //bottom face
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //bottom back left
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //bottom back right
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //bottom front right
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //bottom front left

        //back face
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //bottom back right
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //bottom back left
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }, //top back left
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }, //top back right
    };

    SimpleVertex pyramidVertices[] =
    {
        //base
        { XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, -1.0f) },

        //top
        { XMFLOAT3(0.0f, 3.0f, 0.0f), XMFLOAT3(0.0f, 3.0f, 0.0f) },
    };

    //cube
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cubeVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA cubeInitData;
	ZeroMemory(&cubeInitData, sizeof(cubeInitData));
    cubeInitData.pSysMem = cubeVertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &cubeInitData, &_VertexBuffers[0]);

    if (FAILED(hr))
        return hr;

    //pyramid
    /*D3D11_BUFFER_DESC pbd;
    ZeroMemory(&pbd, sizeof(pbd));
    pbd.Usage = D3D11_USAGE_DEFAULT;
    pbd.ByteWidth = sizeof(pyramidVertices);
    pbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA pyramidInitData;
    ZeroMemory(&pyramidInitData, sizeof(pyramidInitData));
    pyramidInitData.pSysMem = pyramidVertices;

    hr = _pd3dDevice->CreateBuffer(&pbd, &pyramidInitData, &_VertexBuffers[1]);*/

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD cubeIndices[] =
    {
        //front
        0, 1, 2,
        2, 3, 0,
        //right
        4, 5, 6,
        6, 7, 4,
        //top
        8, 9, 10,
        10, 11, 8,
        //left
        12, 13, 14,
        14, 15, 12,
        //bottom
        16, 17, 18,
        18, 19, 16,
        //back
        20, 21, 22,
        22, 23, 20
    };

    WORD pyramidIndices[] =
    {
        0, 2, 1,    // base
        1, 2, 3,
        0, 1, 4,    // sides
        1, 3, 4,
        3, 2, 4,
        2, 0, 4,
    };

    //cube indices

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cubeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA cubeInitData;
	ZeroMemory(&cubeInitData, sizeof(cubeInitData));
    cubeInitData.pSysMem = cubeIndices;
    hr = _pd3dDevice->CreateBuffer(&bd, &cubeInitData, &_cubeIndexBuffer);

    if (FAILED(hr))
        return hr;

    //pyramid indices

    /*D3D11_BUFFER_DESC pbd;
    ZeroMemory(&pbd, sizeof(pbd));

    pbd.Usage = D3D11_USAGE_DEFAULT;
    pbd.ByteWidth = sizeof(pyramidIndices);
    pbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    pbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA pyramidInitData;
    ZeroMemory(&pyramidInitData, sizeof(pyramidInitData));
    pyramidInitData.pSysMem = pyramidIndices;
    hr = _pd3dDevice->CreateBuffer(&pbd, &pyramidInitData, &_pyramidIndexBuffer);*/

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, screenWidth, screenHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"Jacob Dexter | DirectX | Assignment 1", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;
    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, _VertexBuffers, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

   //create rasterizers
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    D3D11_RASTERIZER_DESC srdesc;
    ZeroMemory(&srdesc, sizeof(D3D11_RASTERIZER_DESC));
    srdesc.FillMode = D3D11_FILL_SOLID;
    srdesc.CullMode = D3D11_CULL_BACK;
    srdesc.FrontCounterClockwise = true; //remove when cube no longer needed
    hr = _pd3dDevice->CreateRasterizerState(&srdesc, &_solidRaster);


    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
    if (_VertexBuffers[0]) _VertexBuffers[0]->Release();
    //if (_VertexBuffers[1]) _VertexBuffers[1]->Release();
    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_wireFrame) _wireFrame->Release();
    if (_solidRaster) _solidRaster->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    if (GetAsyncKeyState(VK_F1))
    {
        rasterizerState = !rasterizerState;
    }
    //
    // Pass time through to shaders
    //
    cb.t = t;

    //
    // Animate the cube
    //
	XMStoreFloat4x4(&_world, XMMatrixTranslation(0.0f, 0.0f, 150.0f));

    //spin
    XMStoreFloat4x4(&_world2, XMMatrixRotationY(t * 0.5f) * XMMatrixTranslation(0.0f, 0.0f, 1.0f));
    car->Update(t);
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

    if (rasterizerState)
    {
        _pImmediateContext->RSSetState(_wireFrame);
    }
    else
    {
        _pImmediateContext->RSSetState(_solidRaster);
    }

    //
    // Update variables
    //
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

    //lighting
    cb.LightVecW = lightDirection;

    cb.DiffuseLight = diffuseLight;
    cb.DiffuseMtrl = diffuseMtrl;

    cb.AmbientMtrl = AmbientMtrl;
    cb.AmbientLight = AmbientLight;

    cb.SpecularMtrl = SpecularMtrl;
    cb.SpecularLight = SpecularLight;
    cb.SpecularPower = SpecularPower;

    //camera
    cb.EyePosW = XMFLOAT3(0.0f, 0.0f, -3.0f);

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    ////// Set vertex and index buffers //////
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

    //Renders a cube

    world = XMLoadFloat4x4(&_world2);
    cb.mWorld = XMMatrixTranspose(world);

    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    ////// Set vertex and index buffers //////
    _pImmediateContext->IASetVertexBuffers(0, 1, &_VertexBuffers[0], &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_pImmediateContext->DrawIndexed(36, 0, 0); 

    //draw car
    /*world = XMLoadFloat4x4(&_world);
    cb.mWorld = XMMatrixTranspose(world);

    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    _pImmediateContext->IASetVertexBuffers(0, 1, &objMeshData.VertexBuffer, &objMeshData.VBStride, &objMeshData.VBOffset);
    _pImmediateContext->IASetIndexBuffer(objMeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);*/
    car->Draw();

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}

//HRESULT Application::CalculateNormals(int triCount, SimpleVertex vertexBuffer[], WORD indexBuffer[])
//{
//    int vertexNumber = 0;
//
//    for (int i = 0; i < triCount; i++)
//    {
//        XMVECTOR v1 = XMLoadFloat3(&XMFLOAT3(vertexBuffer[indexBuffer[vertexNumber]].Pos.x, vertexBuffer[indexBuffer[vertexNumber]].Pos.y, vertexBuffer[indexBuffer[vertexNumber]].Pos.z));
//        XMVECTOR v2 = XMLoadFloat3(&XMFLOAT3(vertexBuffer[indexBuffer[vertexNumber + 1]].Pos.x, vertexBuffer[indexBuffer[vertexNumber + 1]].Pos.y, vertexBuffer[indexBuffer[vertexNumber + 1]].Pos.z));
//        XMVECTOR v3 = XMLoadFloat3(&XMFLOAT3(vertexBuffer[indexBuffer[vertexNumber + 2]].Pos.x, vertexBuffer[indexBuffer[vertexNumber + 2]].Pos.y, vertexBuffer[indexBuffer[vertexNumber + 2]].Pos.z));
//        XMVECTOR n = XMVector3Cross(XMVectorSubtract(v2, v1), XMVectorSubtract(v3, v1));
//        vertexNumber += 3;
//    }
//
//    return S_OK;
//}