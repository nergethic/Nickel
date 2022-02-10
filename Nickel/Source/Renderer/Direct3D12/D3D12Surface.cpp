#include "D3D12Surface.h"
#include "D3D12Core.h"

namespace Nickel::Renderer::DX12Layer {
	namespace {
		constexpr auto ToNonSrgb(DXGI_FORMAT format) -> DXGI_FORMAT {
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			return format;
		}
	}

	auto D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format) -> void {
		Assert(factory != nullptr);
		Assert(cmdQueue != nullptr);

		Release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(u32))) && allowTearing) {
			presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		this->format = format;

		DXGI_SWAP_CHAIN_DESC1 desc{
			.Width = window.GetWidth(),
			.Height = window.GetHeight(),
			.Format = ToNonSrgb(format),
			.Stereo = FALSE,
			.SampleDesc = DXGI_SAMPLE_DESC{
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = BUFFER_COUNT,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : static_cast<UINT>(0)
		};

		IDXGISwapChain1* tempSwapChain;
		auto hwnd = reinterpret_cast<HWND>(window.GetHandle());
		ASSERT_ERROR_RESULT(factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, nullptr, nullptr, &tempSwapChain));
		ASSERT_ERROR_RESULT(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)));

		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

		for (u32 i = 0; i < 3; i++)
			renderTargetData[i].rtv = Core::GetRtvHeap().Allocate();

		Finalize();
	}

	auto D3D12Surface::Present() const -> void {
		Assert(swapChain != nullptr);
		ASSERT_ERROR_RESULT(swapChain->Present(0, presentFlags));
		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	auto D3D12Surface::Resize() -> void {
		Assert(swapChain);
		for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
			SafeRelease(renderTargetData[i].resource);
		}

		const u32 flags{ allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0ul };
		ASSERT_ERROR_RESULT(swapChain->ResizeBuffers(BUFFER_COUNT, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

		Finalize();

		//DEBUG_OP(OutputDebugString(L"::D3D12 Surface Resized.\n"));
	}

	auto D3D12Surface::Finalize() -> void {
		for (u32 i = 0; i < BUFFER_COUNT; i++) {
			RenderTargetData& data = renderTargetData[i];
			Assert(data.resource != nullptr);
			swapChain->GetBuffer(i, IID_PPV_ARGS(&data.resource));
			D3D12_RENDER_TARGET_VIEW_DESC desc{
			.Format = format,
			.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D,
			};

			Core::GetDevice()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
		}

		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		const u32 width = desc.BufferDesc.Width;
		const u32 height = desc.BufferDesc.Height;
		Assert(width == window.GetWidth() && height == window.GetHeight());
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<f32>(width);
		viewport.Height = static_cast<f32>(height);
		viewport.MinDepth = 0.1f;
		viewport.MinDepth = 1.0f;
		scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	}

	auto D3D12Surface::Release() -> void {
		for (u32 i = 0; i < BUFFER_COUNT; i++) {
			RenderTargetData& data = renderTargetData[i];
			SafeRelease(data.resource);
			Core::GetRtvHeap().Free(data.rtv);
		}
		
		SafeRelease(swapChain);
	}
}