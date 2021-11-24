#pragma once
#include "../platform.h"
#include "DirectX12Includes.h"
#include "../Window.h"
#include "Resources.h"

namespace Nickel::Renderer::DX12Layer {
	class D3D12Surface {
	public:
		explicit D3D12Surface(Platform::Window window) : window(window) {
			Assert(window.GetHandle());

		}

		~D3D12Surface() {
			Release();
		}

		auto CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format) -> void;
		auto Present() const -> void;
		auto Resize(u32 width, u32 height) -> void;
		auto Finalize() -> void;

		constexpr auto GetWidth() const -> u32 { return static_cast<u32>(viewport.Width); }
		constexpr auto GetHeight() const -> u32 { return static_cast<u32>(viewport.Height); }
		constexpr auto GetCurrentBackbuffer() const -> ID3D12Resource* const { return renderTargetData[currentBackBufferIndex].resource; }
		constexpr auto GetCurrentRtv() const -> D3D12_CPU_DESCRIPTOR_HANDLE { return renderTargetData[currentBackBufferIndex].rtv.cpu; }
		constexpr auto GetViewport() const -> const D3D12_VIEWPORT& { return viewport; }
		constexpr auto GetScissorRect() const -> const D3D12_RECT& { return scissorRect; }
	private:
		auto Release() -> void;

		struct RenderTargetData {
			ID3D12Resource* resource;
			DescriptorHandle rtv{};
		};

		Platform::Window window{};
		IDXGISwapChain4* swapChain = nullptr;
		RenderTargetData renderTargetData[3]{};
		mutable u32 currentBackBufferIndex = 0;
		D3D12_VIEWPORT viewport{};
		D3D12_RECT scissorRect{};
	};
}