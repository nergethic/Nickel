#include "renderer.h"

static UINT MSAA_LEVEL = 4; 

namespace Nickel::Renderer {
	auto Initialize(HWND wndHandle, u32 clientWidth, u32 clientHeight) -> RendererState {
		if (!XMVerifyCPUSupport()) {
			MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
			//return -1; // TODO: logger
		}

		auto [device, deviceCtx] = DXLayer::CreateDevice();

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
		d3dDebug = DXLayer::EnableDebug(*device1);
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
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

		D3D11_VIEWPORT viewport = DXLayer::CreateViewPort(0.0f, 0.0f, backBufferDesc.Width, backBufferDesc.Height);
		deviceCtx1->RSSetViewports(1, &viewport);

		RendererState rs = {
			.g_WindowHandle = wndHandle,
			.device = device1,
			.swapChain = swapChain1,
			.cmdQueue = DXLayer::CmdQueue {
				.queue = deviceCtx1,
				.debug = d3dDebug
			},
			.defaultRenderTargetView = renderTargetView,
			.g_Viewport = viewport,

			.backbufferWidth = backBufferDesc.Width,
			.backbufferHeight = backBufferDesc.Height
		};

		ZeroMemory(rs.zeroBuffer, ArrayCount(rs.zeroBuffer));
		ZeroMemory(rs.zeroSamplerStates, ArrayCount(rs.zeroSamplerStates));
		ZeroMemory(rs.zeroResourceViews, ArrayCount(rs.zeroBuffer));

		return rs;
	}

	auto GetHResultString(HRESULT errCode) -> std::string {
		switch (errCode) {
			// Windows
			case S_OK:           return "S_OK";
			case E_NOTIMPL:      return "E_NOTIMPL";
			case E_NOINTERFACE:  return "E_NOINTERFACE";
			case E_POINTER:      return "E_POINTER";
			case E_ABORT:        return "E_ABORT";
			case E_FAIL:         return "E_FAIL";
			case E_UNEXPECTED:   return "E_UNEXPECTED";
			case E_ACCESSDENIED: return "E_ACCESSDENIED";
			case E_HANDLE:       return "E_HANDLE";
			case E_OUTOFMEMORY:  return "E_OUTOFMEMORY";
			case E_INVALIDARG:   return "E_INVALIDARG";

			// DX11
			case D3D11_ERROR_FILE_NOT_FOUND:                return "D3D11_ERROR_FILE_NOT_FOUND";
			case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";

			// DXGI
			case DXGI_ERROR_ACCESS_DENIED:                return "DXGI_ERROR_ACCESS_DENIED";
			case DXGI_ERROR_ACCESS_LOST:                  return "DXGI_ERROR_ACCESS_LOST";
			case DXGI_ERROR_ALREADY_EXISTS:               return "DXGI_ERROR_ALREADY_EXISTS";
			case DXGI_ERROR_CANNOT_PROTECT_CONTENT:       return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";
			case DXGI_ERROR_DEVICE_HUNG:                  return "DXGI_ERROR_DEVICE_HUNG";
			case DXGI_ERROR_DEVICE_REMOVED:               return "DXGI_ERROR_DEVICE_REMOVED";
			case DXGI_ERROR_DEVICE_RESET:                 return "DXGI_ERROR_DEVICE_RESET";
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:        return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:    return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
			case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
			case DXGI_ERROR_INVALID_CALL:                 return "DXGI_ERROR_INVALID_CALL";
			case DXGI_ERROR_MORE_DATA:                    return "DXGI_ERROR_MORE_DATA";
			case DXGI_ERROR_NAME_ALREADY_EXISTS:          return "DXGI_ERROR_NAME_ALREADY_EXISTS";
			case DXGI_ERROR_NONEXCLUSIVE:                 return "DXGI_ERROR_NONEXCLUSIVE";
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:      return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
			case DXGI_ERROR_NOT_FOUND:                    return "DXGI_ERROR_NOT_FOUND";
			case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:   return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
			case DXGI_ERROR_REMOTE_OUTOFMEMORY:           return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
			case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:     return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";
			case DXGI_ERROR_SDK_COMPONENT_MISSING:        return "DXGI_ERROR_SDK_COMPONENT_MISSING";
			case DXGI_ERROR_SESSION_DISCONNECTED:         return "DXGI_ERROR_SESSION_DISCONNECTED";
			case DXGI_ERROR_UNSUPPORTED:                  return "DXGI_ERROR_UNSUPPORTED";
			case DXGI_ERROR_WAIT_TIMEOUT:                 return "DXGI_ERROR_WAIT_TIMEOUT";
			case DXGI_ERROR_WAS_STILL_DRAWING:            return "DXGI_ERROR_WAS_STILL_DRAWING";

			default: return "Unhandled HRESULT code: " + std::to_string(static_cast<u32>(errCode));
		}

		return "";
	}

	auto GetHResultErrorMessage(HRESULT errCode) -> std::string {
		std::string result = GetHResultString(errCode);
		// TODO: add additional checks for robustness
		// if DXGI_ERROR_DEVICE_REMOVED, DXGI_ERROR_DEVICE_RESET or DXGI_ERROR_DRIVER_INTERNAL_ERROR 
		// ID3D11Device device - GetDeviceRemovedReason();

		return result;
	}
}

template<>
auto GetLatestProfile<ID3D11VertexShader>(RendererState* rs) -> std::string {
	Assert(rs->device);

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
	Assert(rs->device);

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
	Assert(rs->device);
	Assert(pShaderBlob);

	ID3D11VertexShader* pVertexShader = nullptr;
	rs->device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader);

	return pVertexShader;
}

template<>
auto CreateShader<ID3D11PixelShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) -> ID3D11PixelShader* {
	Assert(rs->device);
	Assert(pShaderBlob);

	ID3D11PixelShader* pPixelShader = nullptr;
	rs->device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader);

	return pPixelShader;
}