#include "DX11Layer.h"

namespace Renderer {
	namespace DXLayer {
		auto CreateDevice() -> std::pair<ID3D11Device*, ID3D11DeviceContext*> {
			// Buffer Desc
			/*
			DXGI_MODE_DESC bufferDesc;
			ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

			bufferDesc.Width = GlobalWindowWidth;
			bufferDesc.Height = GlobalWindowHeight;
			bufferDesc.RefreshRate.Numerator = 60;
			bufferDesc.RefreshRate.Denominator = 1;
			bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			*/

			/*
			//Describe our SwapChain
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

			swapChainDesc.BufferCount = 1;
			swapChainDesc.BufferDesc.Width = clientWidth;
			swapChainDesc.BufferDesc.Height = clientHeight;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, false); // TODO vsync
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = rs.g_WindowHandle;
			swapChainDesc.SampleDesc.Count = MSAA_LEVEL;
			//swapChainDesc.SampleDesc.Quality = GetHighestQualitySampleLevel(device, DXGI_FORMAT format);
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Windowed = TRUE;
			*/

			UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
			deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
			// deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUGGABLE; // this shit just gave up and refuses to work (but be sure to have Graphics Tools feature installed on Win 10!)
#endif

			D3D_FEATURE_LEVEL selectedFeatureLevel;

			ID3D11Device* device = nullptr;
			ID3D11DeviceContext* deviceCtx = nullptr;
			HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, FEATURE_LEVELS, ArrayCount(FEATURE_LEVELS), D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &deviceCtx);
			if (result == E_INVALIDARG) { // Create device with feature levels up to 11_0
				result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, &FEATURE_LEVELS[1], ArrayCount(FEATURE_LEVELS) - 1, D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &deviceCtx);
			}
			if (FAILED(result)) {
				//return -1;
				// TODO: Logger
			}

			return { device, deviceCtx };
		}

		auto CreateSwapChain(HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight) -> IDXGISwapChain1* {
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc1 = { 0 };

			swapChainDesc1.Stereo = FALSE;
			swapChainDesc1.BufferCount = 1; // 2;
			swapChainDesc1.Width = clientWidth;
			swapChainDesc1.Height = clientHeight;
			swapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc1.SampleDesc.Count = 4; // TODO use MSAA_LEVEL;
			swapChainDesc1.SampleDesc.Quality = Renderer::GetHighestQualitySampleLevel(device, DXGI_FORMAT_R8G8B8A8_UNORM);
			swapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; // TODO: DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = { 0 };
			swapChainFullscreenDesc.RefreshRate = Renderer::QueryRefreshRate(clientWidth, clientHeight, false);
			swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_CENTERED;
			swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainFullscreenDesc.Windowed = TRUE;

			IDXGIFactory2* pFactory;
			UINT factoryFlags = 0;
#if defined(_DEBUG)
			factoryFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
			HRESULT result = CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&pFactory));
			if (FAILED(result)) {
				//return -1; // TODO: logger
			}

			IDXGISwapChain1* swapChain1 = nullptr;
			result = pFactory->CreateSwapChainForHwnd(device, windowHandle, &swapChainDesc1, &swapChainFullscreenDesc, nullptr, &swapChain1);
			if (FAILED(result) || result == DXGI_ERROR_INVALID_CALL) {
				//return -1; // TODO: logger
			}

			// SafeRelease(pFactory); // TODO
			return swapChain1;
		}
	}
}
