#include "D3D11Core.h"
#include "D3D11Surface.h"

#include "DX11Layer.h"
#include "ForwardPass.h"

namespace Nickel::Renderer::DX11Layer::Core {
	namespace {
		ID3D11Device1* mainDevice = nullptr;
		ID3D11DeviceContext1* cmdList = nullptr;
		CmdQueue cmdQueue{};
		IDXGIFactory2* dxgiFactory = nullptr;
		// debug
		utl::free_list<D3D11Surface> surfaces;

		const static FLOAT g_CLEAR_COLOR[4] = { 0.13333f, 0.13333f, 0.13333f, 1.0f };
	}

	auto Init() -> bool {
		if (mainDevice != nullptr)
			Shutdown();

		auto [device, deviceCtx] = DX11Layer::CreateDevice();

		ID3D11Device1* device1 = nullptr;
		ASSERT_ERROR_RESULT(device->QueryInterface(&device1));
		mainDevice = device1;

		device1->GetImmediateContext1(&cmdList);

		ID3D11Debug* d3dDebug = nullptr;
		if constexpr (_DEBUG) {
			d3dDebug = DX11Layer::EnableDebug(*device1, false);
			ASSERT_ERROR_RESULT(d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_SUMMARY | D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL));
		}

		cmdQueue = CmdQueue{ .queue = cmdList, .debug = d3dDebug };

		UINT factoryFlags = 0;
		if constexpr (_DEBUG) {
			factoryFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
		}
		HRESULT result = CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));
		if (FAILED(result)) {
			Logger::Error("DXGIFactory2 creation failed");
			Assert(false);
		}

		return true;
	}

	auto Shutdown() -> void {

	}

	auto GetDevice() -> ID3D11Device1* const {
		return mainDevice;
	}

	auto GetCmd() -> ID3D11DeviceContext1* const {
		return cmdList;
	}

	auto CreateSurface(Platform::Window window) -> Surface {
		SurfaceId id{ surfaces.add(window) };
		surfaces[id].CreateSwapChain(dxgiFactory, mainDevice);
		return Surface{ id };
		return Surface{ 0 };
	}

	auto RemoveSurface(SurfaceId id) -> void {
		surfaces.remove(id);
	}

	auto ResizeSurface(SurfaceId id, u32 width, u32 height) -> void {
		surfaces[id].Resize();
	}

	auto GetSurfaceWidth(SurfaceId id) -> u32 {
		return surfaces[id].GetWidth();
	}

	auto GetSurfaceHeight(SurfaceId id) -> u32 {
		return surfaces[id].GetHeight();
	}

	auto RenderSurface(SurfaceId id) -> void {
		//cmd.BeginFrame();
		//ID3D12GraphicsCommandList6* cmdList = cmd.GetCmdList();

		//const u32 frameIndex = GetCurrentFrameIndex();
		//if (deferredReleasesFlag[frameIndex]) {
			//ProcessDeferredReleases(frameIndex);
		//}
		//cmd.EndFrame();

		const D3D11Surface& surface = surfaces[id];
		ID3D11Resource* const currentBackBuffer = surface.GetCurrentBackbuffer();

		auto viewport = surface.GetViewport();
		auto scissorRect = surface.GetScissorRect();
		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		auto rtv = surface.GetCurrentRtv();
		auto dsv = surface.GetCurrentDsv();

		cmdList->OMSetRenderTargets(1, &rtv, dsv);

		DX11Layer::ClearFlag clearFlag = DX11Layer::ClearFlag::CLEAR_COLOR | DX11Layer::ClearFlag::CLEAR_DEPTH;
		DX11Layer::Clear(cmdQueue, static_cast<u32>(clearFlag), rtv, dsv, g_CLEAR_COLOR, 1.0f, 0);

		surface.Present();
	}
}