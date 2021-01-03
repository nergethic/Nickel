#include "renderer.h"

void Clear(RendererState* rs, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
	rs->deviceCtx->ClearRenderTargetView(rs->defaultRenderTargetView, clearColor);
	rs->deviceCtx->ClearDepthStencilView(rs->defaultDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

// function inspired by:
// http://www.rastertek.com/dx11tut03.html
DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync)
{
	DXGI_RATIONAL refreshRate = { 0, 1 };
	if (vsync)
	{
		IDXGIFactory* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		DXGI_MODE_DESC* displayModeList;

		// Create a DirectX graphics interface factory.
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		if (FAILED(hr))
		{
			MessageBox(0,
				TEXT("Could not create DXGIFactory instance."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to create DXGIFactory.");
		}

		hr = factory->EnumAdapters(0, &adapter);
		if (FAILED(hr))
		{
			MessageBox(0,
				TEXT("Failed to enumerate adapters."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapters.");
		}

		hr = adapter->EnumOutputs(0, &adapterOutput);
		if (FAILED(hr))
		{
			MessageBox(0,
				TEXT("Failed to enumerate adapter outputs."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to enumerate adapter outputs.");
		}

		UINT numDisplayModes;
		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
		if (FAILED(hr))
		{
			MessageBox(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		assert(displayModeList);

		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
		if (FAILED(hr))
		{
			MessageBox(0,
				TEXT("Failed to query display mode list."),
				TEXT("Query Refresh Rate"),
				MB_OK);

			throw new std::exception("Failed to query display mode list.");
		}

		// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
		for (UINT i = 0; i < numDisplayModes; ++i)
		{
			if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight)
			{
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

ID3D11DepthStencilState* CreateDepthStencilState(ID3D11Device* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest) {
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

ID3D11RasterizerState* CreateRasterizerState(ID3D11Device* device) {
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

ID3D11Texture2D* CreateDepthStencilTexture(ID3D11Device* device, UINT width, UINT height) {
	assert(device != nullptr);
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
	ZeroMemory(&depthStencilTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthStencilTextureDesc.ArraySize = 1;
	depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilTextureDesc.CPUAccessFlags = 0; // No CPU access required.
	depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilTextureDesc.Width = width;
	depthStencilTextureDesc.Height = height;
	depthStencilTextureDesc.MipLevels = 1;
	depthStencilTextureDesc.SampleDesc.Count = 8;
	depthStencilTextureDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
	depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	HRESULT hr = device->CreateTexture2D(&depthStencilTextureDesc, nullptr, &depthStencilBuffer);
	if (FAILED(hr)) {
		// TODO: report error
	}

	return depthStencilBuffer;
}

ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Device* device, ID3D11Resource* depthStencilTexture) {
	assert(device != nullptr);
	assert(depthStencilTexture != nullptr);

	// D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	
	ID3D11DepthStencilView* result = nullptr;
	device->CreateDepthStencilView(depthStencilTexture, nullptr, &result);
	if (FAILED(result)) {
		// TODO: report error
	}

	return result;
}

i32 CreateVertexBuffer(RendererState* rs, u32 size) {
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = size;
	bufferDesc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_FLAG;
	bufferDesc.MiscFlags = 0; // D3D11_RESOURCE_MISC_FLAG
	bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

	i32 bufferIndex = rs->vertexBuffersCount;
	HRESULT hr = rs->device->CreateBuffer(&bufferDesc, NULL, &rs->vertexBuffers[bufferIndex]);
	if (!SUCCEEDED(hr)) {
		bufferIndex = -1;
	}

	rs->vertexBuffersCount++;

	return bufferIndex;
}