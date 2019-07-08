#include "renderer.h"

void Clear(RendererState* rs, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
	rs->deviceCtx->ClearRenderTargetView(rs->g_d3dRenderTargetView, clearColor);
	rs->deviceCtx->ClearDepthStencilView(rs->g_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
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