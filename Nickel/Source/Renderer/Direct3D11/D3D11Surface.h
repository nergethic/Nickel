#pragma once
#include "../platform.h"
#include "../Window.h"
#include "DirectX11Includes.h"
//#include "Resources.h"

namespace Nickel::Renderer::DX11Layer {
	class D3D11Surface {
	public:
		constexpr static DXGI_FORMAT DEFAULT_BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;  // TODO: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
		constexpr static u32 BUFFER_COUNT = 1;

		explicit D3D11Surface(Platform::Window window) : window(window) {
			Assert(window.GetHandle() != nullptr);
		}

		~D3D11Surface() {
			Release();
		}

#if USE_STL_VECTOR
		explicit D3D11Surface(const D3D11Surface&) = delete;
		D3D11Surface& operator=(const D3D11Surface&) = delete;
		constexpr D3D11Surface(D3D11Surface&& o)
			: swapChain{ o.swapChain }, window{ o.window }, currentBackBufferIndex{ o.currentBackBufferIndex }
			, viewport{ o.viewport }, scissorRect{ o.scissorRect }, allowTearing{ o.allowTearing }
			, presentFlags{ o.presentFlags }
		{
			for (u32 i{ 0 }; i < BUFFER_COUNT; ++i) {
				renderTargetData[i].resource = o.renderTargetData[i].resource;
				renderTargetData[i].rtv = o.renderTargetData[i].rtv;
				renderTargetData[i].depthStencilResource = o.renderTargetData[i].depthStencilResource;
				renderTargetData[i].dsv = o.renderTargetData[i].dsv;
			}

			o.Reset();
		}

		constexpr D3D11Surface& operator=(D3D11Surface&& o) {
			Assert(this != &o);
			if (this != &o) {
				Release();
				Move(o);
			}

			return *this;
		}
#else
		explicit D3D11Surface(const D3D11Surface&) = delete;
		D3D11Surface& operator=(const D3D11Surface&) = delete;
		explicit D3D11Surface(D3D11Surface&&) = delete;
		D3D11Surface& operator=(D3D11Surface&&) = delete;
#endif // USE_STL_VECTOR

		auto CreateSwapChain(IDXGIFactory2* factory, ID3D11Device1* device, DXGI_FORMAT format = DEFAULT_BACK_BUFFER_FORMAT) -> void;
		auto Present() const -> void;
		auto Resize() -> void;

		constexpr auto GetWidth() const -> u32 { return static_cast<u32>(viewport.Width); }
		constexpr auto GetHeight() const -> u32 { return static_cast<u32>(viewport.Height); }
		constexpr auto GetCurrentBackbuffer() const -> ID3D11Resource* const { return renderTargetData[currentBackBufferIndex].resource; }
		constexpr auto GetCurrentRtv() const -> ID3D11RenderTargetView* { return renderTargetData[currentBackBufferIndex].rtv; }
		constexpr auto GetCurrentDepthStencil() const -> ID3D11Resource* const { return renderTargetData[currentBackBufferIndex].depthStencilResource; }
		constexpr auto GetCurrentDsv() const -> ID3D11DepthStencilView* { return renderTargetData[currentBackBufferIndex].dsv; }
		constexpr auto GetViewport() const -> const D3D11_VIEWPORT& { return viewport; }
		constexpr auto GetScissorRect() const -> const D3D11_RECT& { return scissorRect; }
	private:
		auto Finalize() -> void;
		auto Release() -> void;

		struct RenderTargetData {
			ID3D11Resource* resource = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;

			ID3D11Resource* depthStencilResource = nullptr;
			ID3D11DepthStencilView* dsv;
		};

#if USE_STL_VECTOR
		constexpr void Move(D3D11Surface& o) {
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

		IDXGISwapChain1* swapChain = nullptr;
		RenderTargetData renderTargetData[BUFFER_COUNT]{};
		Platform::Window window{};
		DXGI_FORMAT format{ DEFAULT_BACK_BUFFER_FORMAT };
		mutable u32 currentBackBufferIndex = 0;
		u32 allowTearing = 0;
		u32 presentFlags = 0;
		D3D11_VIEWPORT viewport{};
		D3D11_RECT scissorRect{};
	};
}