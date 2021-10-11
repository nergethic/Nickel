#include "renderer.h"

static UINT MSAA_LEVEL = 4;

auto Renderer::CreateViewPort(f32 minX, f32 minY, f32 maxX, f32 maxY) -> D3D11_VIEWPORT {
	return {
		.TopLeftX = minX,
		.TopLeftY = minY,
		.Width  = static_cast<FLOAT>(maxX),
		.Height = static_cast<FLOAT>(maxY),
		.MinDepth = D3D11_MIN_DEPTH,
		.MaxDepth = D3D11_MAX_DEPTH
	};
}

auto Renderer::EnableDebug(const ID3D11Device1& device1) -> ID3D11Debug* {
	ID3D11Debug* d3dDebug {};

	auto d = const_cast<ID3D11Device1*>(&device1);
	auto result = d->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(result)) {
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&d3dInfoQueue)))) {

			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			// d3dInfoQueue->Release();
		}

		// d3dDebug->Release();
	}

	return d3dDebug;
}

auto Renderer::Initialize(HWND wndHandle, u32 clientWidth, u32 clientHeight) -> RendererState {
	if (!XMVerifyCPUSupport()) {
		MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
		//return -1; // TODO: logger
	}

	auto [device, deviceCtx] = Renderer::DXLayer::CreateDevice();

	ID3D11Device1* device1 = nullptr;
	HRESULT result = device->QueryInterface(&device1);
	if (FAILED(result)) {
		int x = 4;
		//return -1; // TODO: logger
	}

	ID3D11DeviceContext1* deviceCtx1 = nullptr;
	device1->GetImmediateContext1(&deviceCtx1);

	ID3D11Debug* d3dDebug = nullptr;
#if defined(_DEBUG)
	//d3dDebug = EnableDebug(*device1);
#endif

	auto swapChain1 = Renderer::DXLayer::CreateSwapChain(wndHandle, device1, clientWidth, clientHeight);

	// back buffer for swap chain
	ID3D11Texture2D* backBufferTexture;
	result = swapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(result)) {
		int x = 4;
		//return -1; // TODO: logger
	}

	// D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	ID3D11RenderTargetView* renderTargetView;
	result = device1->CreateRenderTargetView(backBufferTexture, NULL, &renderTargetView);
	if (FAILED(result)) {
		int x = 4;
		//return -1; // TODO: logger
	}
	deviceCtx1->OMSetRenderTargets(1, &renderTargetView, NULL); // test code

	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	backBufferTexture->GetDesc(&backBufferDesc);
	SafeRelease(backBufferTexture);

	D3D11_VIEWPORT viewport = CreateViewPort(0.0f, 0.0f, backBufferDesc.Width, backBufferDesc.Height);
	deviceCtx1->RSSetViewports(1, &viewport);

	RendererState rs = {
		.g_WindowHandle = wndHandle,
		.device = device1,
		.deviceCtx = deviceCtx1,
		.swapChain = swapChain1,
		.defaultRenderTargetView = renderTargetView,
		.g_Viewport = viewport,

		.backbufferWidth = backBufferDesc.Width,
		.backbufferHeight = backBufferDesc.Height,
		.d3dDebug = d3dDebug
	};

	ZeroMemory(rs.zeroBuffer,        ArrayCount(rs.zeroBuffer));
	ZeroMemory(rs.zeroSamplerStates, ArrayCount(rs.zeroSamplerStates));
	ZeroMemory(rs.zeroResourceViews, ArrayCount(rs.zeroBuffer));

	return rs;
}

auto Renderer::Clear(RendererState* rs, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void {
	rs->deviceCtx->ClearRenderTargetView(rs->defaultRenderTargetView, clearColor);
	rs->deviceCtx->ClearDepthStencilView(rs->defaultDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

// function inspired by:
// http://www.rastertek.com/dx11tut03.html
auto Renderer::QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync) -> DXGI_RATIONAL {
	DXGI_RATIONAL refreshRate = { 0, 1 };
	if (vsync) {
		IDXGIFactory2* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		DXGI_MODE_DESC* displayModeList;

		HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&factory);
		if (FAILED(hr)) {
			MessageBox(0,
				TEXT("Could not create DXGIFactory instance."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to create DXGIFactory.");
		}

		hr = factory->EnumAdapters(0, &adapter);
		if (FAILED(hr)) {
			MessageBox(0,
				TEXT("Failed to enumerate adapters."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapters.");
		}

		hr = adapter->EnumOutputs(0, &adapterOutput);
		if (FAILED(hr)) {
			MessageBox(0,
				TEXT("Failed to enumerate adapter outputs."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapter outputs.");
		}

		UINT numDisplayModes;
		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
		if (FAILED(hr)) {
			MessageBox(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		assert(displayModeList);

		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
		if (FAILED(hr)) {
			MessageBox(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
		for (UINT i = 0; i < numDisplayModes; ++i) {
			if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight) {
				refreshRate = displayModeList[i].RefreshRate;
			}
		}

		delete[] displayModeList;
		SafeRelease(adapterOutput);
		SafeRelease(adapter);
		SafeRelease(factory);
	}

	return refreshRate;
}

auto Renderer::CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest) -> ID3D11DepthStencilState* {
	assert(device != nullptr);
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStencilStateDesc.DepthEnable = enableDepthTest ? TRUE : FALSE;
	depthStencilStateDesc.DepthWriteMask = depthWriteMask;
	depthStencilStateDesc.DepthFunc = depthFunc;
	depthStencilStateDesc.StencilEnable = enableStencilTest ? TRUE : FALSE;

	ID3D11DepthStencilState* result = nullptr;
	HRESULT hr = device->CreateDepthStencilState(&depthStencilStateDesc, &result);
	if (!SUCCEEDED(hr)) {
		// TODO: report error
	}

	return result;
}

auto Renderer::CreateDefaultRasterizerState(ID3D11Device1* device) -> ID3D11RasterizerState* {
	assert(device != nullptr);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	ID3D11RasterizerState* result = nullptr;
	HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, &result);
	if (FAILED(result)) {
		// TODO: report error
	}

	return result;
}

auto Renderer::GetHighestQualitySampleLevel(ID3D11Device1* device, DXGI_FORMAT format) -> UINT {
	UINT maxQualityLevelPlusOne;
	device->CheckMultisampleQualityLevels(format, 8, &maxQualityLevelPlusOne); // const_cast<ID3D11Device1&>(

	return maxQualityLevelPlusOne - 1;
}

auto Renderer::CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels = 1) -> ID3D11Texture2D* {
	assert(device != nullptr);
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc = {0};

	depthStencilTextureDesc.ArraySize = 1;
	depthStencilTextureDesc.BindFlags = bindFlags;
	depthStencilTextureDesc.CPUAccessFlags = 0; // No CPU access required.
	depthStencilTextureDesc.Format = format;
	depthStencilTextureDesc.Width = width;
	depthStencilTextureDesc.Height = height;
	depthStencilTextureDesc.MipLevels = mipLevels;
	depthStencilTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

	if (mipLevels == 1) {
		depthStencilTextureDesc.SampleDesc.Count = 4; // TODO: match depth stencil count and quality with swap chain settings
		depthStencilTextureDesc.SampleDesc.Quality = GetHighestQualitySampleLevel(device, format);
	} else {
		depthStencilTextureDesc.SampleDesc.Count = 1;
		depthStencilTextureDesc.SampleDesc.Quality = 0;
	}

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	HRESULT hr = device->CreateTexture2D(&depthStencilTextureDesc, nullptr, &depthStencilBuffer);
	if (FAILED(hr)) {
		// TODO: report error
	}

	return depthStencilBuffer;
}

auto Renderer::CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height) -> ID3D11Texture2D* {
	return CreateTexture(device, width, height, DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL);
}

auto Renderer::CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture) -> ID3D11DepthStencilView* {
	assert(device != nullptr);
	assert(depthStencilTexture != nullptr);
	
	// D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	if (device->CreateDepthStencilView(depthStencilTexture, nullptr, nullptr) != S_FALSE) {
		// TODO: report error
		return nullptr;
	}
	
	ID3D11DepthStencilView* result = nullptr;
	device->CreateDepthStencilView(depthStencilTexture, nullptr, &result);
	if (FAILED(result)) {
		// TODO: report error
	}

	return result;
}

auto Renderer::CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
	assert(device != nullptr);

	D3D11_BUFFER_DESC bufferDesc = {0};
	bufferDesc.Usage = usage;
	bufferDesc.BindFlags = bindFlags;
	bufferDesc.ByteWidth = byteWidthSize;
	bufferDesc.CPUAccessFlags = cpuAccessFlags; // D3D11_CPU_ACCESS_FLAG;
	bufferDesc.MiscFlags = miscFlags;           // D3D11_RESOURCE_MISC_FLAG
	// bufferDesc.StructureByteStride

	ID3D11Buffer* newBuffer = nullptr;
	HRESULT hr = device->CreateBuffer(&bufferDesc, initialData, &newBuffer);
	if (!SUCCEEDED(hr)) {
		// TODO: report error
		return nullptr;
	}

	return newBuffer;
}

auto Renderer::CreateVertexBuffer(RendererState* rs, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
	assert(rs != nullptr);
	assert(rs->device != nullptr);

	auto device = rs->device.Get();
	auto newVertexBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, size, 0, 0, initialData);
	//i32 newVertexBufferIndex = rs->vertexBuffersCount; // TODO: if vertex buffer creation fails this still returns index to empty array 
	//if (newVertexBuffer != nullptr) {
		//rs->vertexBuffers[newVertexBufferIndex] = newVertexBuffer;
		//rs->vertexBuffersCount++;
	//}

	return newVertexBuffer;
}

auto Renderer::CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
	assert(device != nullptr);
	auto newIndexBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER, size, 0, 0, initialData);

	return newIndexBuffer;
}

auto Renderer::CreateConstantBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
	assert(device != nullptr);
	auto newConstantBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, size, 0, 0, initialData);

	return newConstantBuffer;
}

auto Renderer::DrawIndexed(ID3D11DeviceContext1* deviceCtx, int indexCount, int startIndex, int startVertex) -> void {
	deviceCtx->DrawIndexed(indexCount, startIndex, startVertex);
}

auto Renderer::CreateInputLayout(ID3D11Device1* device, D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc, UINT vertexLayoutDescLength, const BYTE* shaderBytecodeWithInputSignature, SIZE_T shaderBytecodeSize) -> ID3D11InputLayout* {
	assert(device != nullptr);
	assert(vertexLayoutDesc != nullptr);
	assert(shaderBytecodeWithInputSignature != nullptr);

	ID3D11InputLayout* result = nullptr;

	HRESULT hr = device->CreateInputLayout(
		vertexLayoutDesc,
		vertexLayoutDescLength,
		shaderBytecodeWithInputSignature,
		shaderBytecodeSize,
		&result);

	if (FAILED(hr)) {
		// TODO: log error
		assert(nullptr);
	}

	return result;
}

template<>
auto GetLatestProfile<ID3D11VertexShader>(RendererState* rs) -> std::string {
	assert(rs->device);

	// Query the current feature level:
	D3D_FEATURE_LEVEL featureLevel = rs->device->GetFeatureLevel();

	switch (featureLevel) {
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0: {
		return "vs_5_0";
	} break;

	case D3D_FEATURE_LEVEL_10_1: {
		return "vs_4_1";
	} break;

	case D3D_FEATURE_LEVEL_10_0: {
		return "vs_4_0";
	} break;

	case D3D_FEATURE_LEVEL_9_3: {
		return "vs_4_0_level_9_3";
	} break;

	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1: {
		return "vs_4_0_level_9_1";
	} break;
	}

	return "";
}

template<>
auto GetLatestProfile<ID3D11PixelShader>(RendererState* rs) -> std::string {
	assert(rs->device);

	// Query the current feature level:
	D3D_FEATURE_LEVEL featureLevel = rs->device->GetFeatureLevel();
	switch (featureLevel) {
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0: {
		return "ps_5_0";
	} break;

	case D3D_FEATURE_LEVEL_10_1: {
		return "ps_4_1";
	} break;

	case D3D_FEATURE_LEVEL_10_0: {
		return "ps_4_0";
	} break;

	case D3D_FEATURE_LEVEL_9_3: {
		return "ps_4_0_level_9_3";
	} break;

	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1: {
		return "ps_4_0_level_9_1";
	} break;
	}
	return "";
}

template<>
auto CreateShader<ID3D11VertexShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) -> ID3D11VertexShader* {
	assert(rs->device);
	assert(pShaderBlob);

	ID3D11VertexShader* pVertexShader = nullptr;
	rs->device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader);

	return pVertexShader;
}

template<>
auto CreateShader<ID3D11PixelShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) -> ID3D11PixelShader* {
	assert(rs->device);
	assert(pShaderBlob);

	ID3D11PixelShader* pPixelShader = nullptr;
	rs->device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader);

	return pPixelShader;
}