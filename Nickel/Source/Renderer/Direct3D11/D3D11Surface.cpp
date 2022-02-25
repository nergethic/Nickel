#include "D3D11Surface.h"
#include "D3D11Core.h"

#include "DX11Layer.h"

namespace Nickel::Renderer::DX11Layer {
	namespace {
	}

	auto D3D11Surface::CreateSwapChain(IDXGIFactory2* factory, ID3D11Device1* device, DXGI_FORMAT format) -> void {
		Assert(factory != nullptr);
		Assert(device != nullptr);

		Release();

		this->format = format;
		// HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight) -> IDXGISwapChain1*

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc1 = { 0 };

		auto const clientWidth = window.GetWidth();
		auto const clientHeight = window.GetHeight();

		swapChainDesc1.Stereo = FALSE;
		swapChainDesc1.BufferCount = 1; // TODO: 2;
		swapChainDesc1.Width = clientWidth,
		swapChainDesc1.Height = clientHeight;
		swapChainDesc1.Format = format;
		swapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc1.SampleDesc.Count = 4; // TODO use MSAA_LEVEL;
		swapChainDesc1.SampleDesc.Quality = GetHighestQualitySampleLevel(device, format);
		swapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; // TODO: DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = { 0 };
		swapChainFullscreenDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, false);
		swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_CENTERED;
		swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainFullscreenDesc.Windowed = TRUE;

		auto hwnd = reinterpret_cast<HWND>(window.GetHandle());
		ASSERT_ERROR_RESULT(factory->CreateSwapChainForHwnd(device, hwnd, &swapChainDesc1, &swapChainFullscreenDesc, nullptr, &swapChain));
		ASSERT_ERROR_RESULT(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		Finalize();

		// SafeRelease(pFactory); // TODO

		/*
		// back buffer for swap chain
		ID3D11Texture2D* backBufferTexture;
		ASSERT_ERROR_RESULT(swapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture));

		// D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		ID3D11RenderTargetView* renderTargetView;
		ASSERT_ERROR_RESULT(device1->CreateRenderTargetView(backBufferTexture, NULL, &renderTargetView));

		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		backBufferTexture->GetDesc(&backBufferDesc);
		SafeRelease(backBufferTexture);

		D3D11_VIEWPORT viewport = DX11Layer::CreateViewPort(0.0f, 0.0f, backBufferDesc.Width, backBufferDesc.Height);

		RendererState rs = {
			.g_WindowHandle = wndHandle,
			.device = device1,
			.swapChain = swapChain1,
			.cmdQueue = DX11Layer::CmdQueue {
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
		*/
	}

	auto D3D11Surface::Present() const -> void {
		Assert(swapChain != nullptr);
		ASSERT_ERROR_RESULT(swapChain->Present(0, presentFlags));
		currentBackBufferIndex = 0; // TODO: swapChain->GetCurrentBackBufferIndex();
	}

	auto D3D11Surface::Resize() -> void {
		Assert(swapChain);
		for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
			SafeRelease(renderTargetData[i].resource);
			SafeRelease(renderTargetData[i].depthStencilResource);
		}

		const u32 flags{ allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0ul };
		ASSERT_ERROR_RESULT(swapChain->ResizeBuffers(BUFFER_COUNT, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		currentBackBufferIndex = 0; // TODO:  swapChain->GetCurrentBackBufferIndex();

		Finalize();

		//DEBUG_OP(OutputDebugString(L"::D3D12 Surface Resized.\n"));
	}

	auto D3D11Surface::Finalize() -> void {
		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		const u32 width = desc.BufferDesc.Width;
		const u32 height = desc.BufferDesc.Height;
		Assert(width == window.GetWidth() && height == window.GetHeight());

		auto device = Core::GetDevice();

		for (u32 i = 0; i < BUFFER_COUNT; i++) {
			RenderTargetData& data = renderTargetData[i];
			D3D11_RENDER_TARGET_VIEW_DESC desc{
				.Format = format,
				.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DMS
			};

			Assert(data.resource == nullptr)

			swapChain->GetBuffer(i, IID_PPV_ARGS(&data.resource));

			device->CreateRenderTargetView(data.resource, &desc, &data.rtv);
			data.depthStencilResource = DX11Layer::CreateDepthStencilTexture(device, width, height);
			data.dsv = DX11Layer::CreateDepthStencilView(device, data.depthStencilResource);
		}

		viewport = DX11Layer::CreateViewPort(0.0f, 0.0f, static_cast<f32>(width), static_cast<f32>(height));
		scissorRect = D3D11_RECT{ 0, 0, static_cast<i32>(width), static_cast<i32>(height) };
	}

	auto D3D11Surface::Release() -> void {
		for (u32 i = 0; i < BUFFER_COUNT; i++) {
			RenderTargetData& data = renderTargetData[i];
			SafeRelease(data.resource);
			SafeRelease(data.rtv);
			SafeRelease(data.depthStencilResource);
			SafeRelease(data.dsv);
		}

		SafeRelease(swapChain);
	}
}