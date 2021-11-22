#include "renderer.h"
#include "RendererPlatformInterface.h"
#include "Direct3D11/D3D11Interface.h"
#include "Direct3D12/D3D12Interface.h"

static UINT MSAA_LEVEL = 4; // TODO: move this to config file

namespace Nickel::Renderer {
	namespace {
		PlatformInterface gfx{};
		auto SetPlatformInterface(GraphicsPlatform platform) -> bool {
			switch (platform) {
			case GraphicsPlatform::Direct3D11: {
				DXLayer11::GetPlatformInterface(gfx);
			} break;

					
			case GraphicsPlatform::Direct3D12: {
				DX12Layer::GetPlatformInterface(gfx);
			} break;

			default:
				return false;
			}

			return true;
		}
	}

	auto Init(GraphicsPlatform platform) -> bool {
		Logger::Init(); // TODO: super ugly, figure out where to initialize this
		return SetPlatformInterface(platform) && gfx.Init();
	}

	auto Render() -> void {
		gfx.Render();
	}

	auto Shutdown() -> void {
		gfx.Shutdown();
	}

	auto Initialize(HWND wndHandle, u32 clientWidth, u32 clientHeight) -> RendererState {
		if (!XMVerifyCPUSupport()) {
			MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
			Logger::Critical("XMVerifyCPUSupport failed");
			Assert(false);
		}

		auto [device, deviceCtx] = DXLayer11::CreateDevice();

		ID3D11Device1* device1 = nullptr;
		ASSERT_ERROR_RESULT(device->QueryInterface(&device1));

		ID3D11DeviceContext1* deviceCtx1 = nullptr;
		device1->GetImmediateContext1(&deviceCtx1);

		ID3D11Debug* d3dDebug = nullptr;
		if constexpr (_DEBUG) {
			d3dDebug = DXLayer11::EnableDebug(*device1, false);
			ASSERT_ERROR_RESULT(d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_SUMMARY | D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL));
		}

		auto swapChain1 = Renderer::DXLayer11::CreateSwapChain(wndHandle, device1, clientWidth, clientHeight);

		// back buffer for swap chain
		ID3D11Texture2D* backBufferTexture;
		ASSERT_ERROR_RESULT(swapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture));

		// D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		ID3D11RenderTargetView* renderTargetView;
		ASSERT_ERROR_RESULT(device1->CreateRenderTargetView(backBufferTexture, NULL, &renderTargetView));

		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		backBufferTexture->GetDesc(&backBufferDesc);
		SafeRelease(backBufferTexture);

		D3D11_VIEWPORT viewport = DXLayer11::CreateViewPort(0.0f, 0.0f, backBufferDesc.Width, backBufferDesc.Height);

		RendererState rs = {
			.g_WindowHandle = wndHandle,
			.device = device1,
			.swapChain = swapChain1,
			.cmdQueue = DXLayer11::CmdQueue {
				.queue = deviceCtx1,
				.debug = d3dDebug
			},
			.defaultRenderTargetView = renderTargetView,
			.g_Viewport = viewport,

			.backbufferWidth = backBufferDesc.Width,
			.backbufferHeight = backBufferDesc.Height
		};

		ZeroMemory(rs.zeroBuffer,        ArrayCount(rs.zeroBuffer));
		ZeroMemory(rs.zeroSamplerStates, ArrayCount(rs.zeroSamplerStates));
		ZeroMemory(rs.zeroResourceViews, ArrayCount(rs.zeroBuffer));

		return rs;
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
	ASSERT_ERROR_RESULT(rs->device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader));

	return pVertexShader;
}

template<>
auto CreateShader<ID3D11PixelShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) -> ID3D11PixelShader* {
	Assert(rs->device);
	Assert(pShaderBlob);

	ID3D11PixelShader* pPixelShader = nullptr;
	ASSERT_ERROR_RESULT(rs->device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader));

	return pPixelShader;
}