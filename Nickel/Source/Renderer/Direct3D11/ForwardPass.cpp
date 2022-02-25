#include "ForwardPass.h"
#include "D3D11Core.h"

namespace Nickel::Renderer::DX11Layer::MainPass {
	namespace {
		auto CreateBuffers() -> bool {

			auto device = Core::GetDevice();

			// back buffer for swap chain
			//ID3D11Texture2D* backBufferTexture;
			//ASSERT_ERROR_RESULT(swapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture));

			// D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			//ID3D11RenderTargetView* renderTargetView;
			//ASSERT_ERROR_RESULT(device->CreateRenderTargetView(backBufferTexture, NULL, &renderTargetView));

			//D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
			//backBufferTexture->GetDesc(&backBufferDesc);
			//SafeRelease(backBufferTexture);

			//D3D11_VIEWPORT viewport = DX11Layer::CreateViewPort(0.0f, 0.0f, backBufferDesc.Width, backBufferDesc.Height);

			/*
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
			*/



			//ZeroMemory(rs.zeroBuffer,        ArrayCount(rs.zeroBuffer));
			//ZeroMemory(rs.zeroSamplerStates, ArrayCount(rs.zeroSamplerStates));
			//ZeroMemory(rs.zeroResourceViews, ArrayCount(rs.zeroBuffer));
			return true;
		}
	}

	auto Initialize() -> bool {
		return CreateBuffers();
	}

	auto Shutdown() -> void {

	}

	auto Render() -> void {

	}
}