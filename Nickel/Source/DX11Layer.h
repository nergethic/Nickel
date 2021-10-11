#pragma once
#include "platform.h"
#include "renderer.h"

namespace Renderer {
	namespace DXLayer {
		const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		std::pair<ID3D11Device*, ID3D11DeviceContext*> CreateDevice();
		IDXGISwapChain1* CreateSwapChain(HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight);
	}
}