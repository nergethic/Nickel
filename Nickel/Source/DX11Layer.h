#pragma once

#pragma warning(push, 0) // ignores warnings from external headers
// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib") // TODO: check why this doesn't have stuff from dxgi1_3.h (CreateDXGIFactory2)
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

// DirectX includes
#include "platform.h"
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <WICTextureLoader.h>
#include <dxgi.h>
#include <dxgi1_3.h>

#include <wrl/client.h>
#pragma warning(pop)

#include "platform.h"

using namespace Microsoft::WRL;

namespace Renderer {
	namespace DXLayer {
		struct CmdQueue {
			ComPtr<ID3D11DeviceContext1> queue;
			ComPtr<ID3D11Debug> debug;
		};

		const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		auto QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync) -> DXGI_RATIONAL;
		auto GetHighestQualitySampleLevel(ID3D11Device1* device, DXGI_FORMAT format) -> UINT;
		auto CreateDevice() -> std::pair<ID3D11Device*, ID3D11DeviceContext*>;
		auto CreateSwapChain(HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight) -> IDXGISwapChain1*;
	}
}