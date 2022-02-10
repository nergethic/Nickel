#pragma once
#include "../platform.h"
#include "DirectX12Includes.h"
#include "../Window.h"
#include "Resources.h"

namespace Nickel::Renderer::DX12Layer {
	class D3D12Surface {
	public:
		constexpr static DXGI_FORMAT DEFAULT_BACK_BUFFER_FORMAT { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
		constexpr static u32 BUFFER_COUNT {3};

		explicit D3D12Surface(Platform::Window window) : window(window) {
			Assert(window.GetHandle());
		}

		~D3D12Surface() {
			Release();
		}

#if USE_STL_VECTOR
		explicit D3D12Surface(const D3D12Surface&) = delete;
		D3D12Surface& operator=(const D3D12Surface&) = delete;
		constexpr D3D12Surface(D3D12Surface&& o)
			: swapChain{ o.swapChain }, window{ o.window }, currentBackBufferIndex{ o.currentBackBufferIndex }
			, viewport{ o.viewport }, scissorRect{ o.scissorRect }, allowTearing{ o.allowTearing }
			, presentFlags{ o.presentFlags }
		{
			for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
				renderTargetData[i].resource = o.renderTargetData[i].resource;
				renderTargetData[i].rtv = o.renderTargetData[i].rtv;
			}

			o.Reset();
		}

		constexpr D3D12Surface& operator=(D3D12Surface&& o) {
			Assert(this != &o);
			if (this != &o) {
				Release();
				Move(o);
			}

			return *this;
		}
#else
		explicit D3D12Surface(const D3D12Surface&) = delete;
		D3D12Surface& operator=(const D3D12Surface&) = delete;
		explicit D3D12Surface(D3D12Surface&&) = delete;
		D3D12Surface& operator=(D3D12Surface&&) = delete;
#endif // USE_STL_VECTOR

		auto CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue, DXGI_FORMAT format = DEFAULT_BACK_BUFFER_FORMAT) -> void;
		auto Present() const -> void;
		auto Resize() -> void;

		constexpr auto GetWidth() const -> u32 { return static_cast<u32>(viewport.Width); }
		constexpr auto GetHeight() const -> u32 { return static_cast<u32>(viewport.Height); }
		constexpr auto GetCurrentBackbuffer() const -> ID3D12Resource* const { return renderTargetData[currentBackBufferIndex].resource; }
		constexpr auto GetCurrentRtv() const -> D3D12_CPU_DESCRIPTOR_HANDLE { return renderTargetData[currentBackBufferIndex].rtv.cpu; }
		constexpr auto GetViewport() const -> const D3D12_VIEWPORT& { return viewport; }
		constexpr auto GetScissorRect() const -> const D3D12_RECT& { return scissorRect; }
	private:
		auto Finalize() -> void;
		auto Release() -> void;

		struct RenderTargetData {
			ID3D12Resource* resource;
			DescriptorHandle rtv{};
		};

#if USE_STL_VECTOR
		constexpr void Move(D3D12Surface& o) {
			swapChain = o.swapChain;
			for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
				renderTargetData[i] = o.renderTargetData[i];
			}
			window = o.window;
			currentBackBufferIndex = o.currentBackBufferIndex;
			allowTearing = o.allowTearing;
			presentFlags = o.presentFlags;
			viewport = o.viewport;
			scissorRect = o.scissorRect;

			o.Reset();
		}

		constexpr void Reset() {
			swapChain = nullptr;
			for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
				renderTargetData[i] = {};
			}
			window = {};
			currentBackBufferIndex = 0;
			allowTearing = 0;
			presentFlags = 0;
			viewport = {};
			scissorRect = {};
		}
#endif // USE_STL_VECTOR

		IDXGISwapChain4* swapChain = nullptr;
		RenderTargetData renderTargetData[BUFFER_COUNT]{};
		Platform::Window window{};
		DXGI_FORMAT format{ DEFAULT_BACK_BUFFER_FORMAT };
		mutable u32 currentBackBufferIndex = 0;
		u32 allowTearing = 0;
		u32 presentFlags = 0;
		D3D12_VIEWPORT viewport{};
		D3D12_RECT scissorRect{};
	};
}