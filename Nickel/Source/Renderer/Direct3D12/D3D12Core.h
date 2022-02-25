#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer {
	class DescriptorHeap;
}

namespace Nickel::Renderer::DX12Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;

	namespace Internal {
		auto DeferredRelease(IUnknown* ptr) -> void;
	}

	template<typename T>
	inline auto SafeDeferredRelease(T& ptr) -> void {
		if (ptr != nullptr) {
			Internal::DeferredRelease(ptr);
			ptr = nullptr;
		}
	};

	auto GetDevice()  -> ID3D12Device* const;
	auto GetRtvHeap() -> DescriptorHeap&;
	auto GetDsvHeap() -> DescriptorHeap&;
	auto GetSrvHeap() -> DescriptorHeap&;
	auto GetUavHeap() -> DescriptorHeap&;
	auto GetDefaultRenderTargetFormat() -> DXGI_FORMAT;
	auto GetCurrentFrameIndex() -> u32;
	auto SetDeferredReleasesFlag() -> void;

	auto CreateSurface(Platform::Window window) -> Surface;
	auto RemoveSurface(SurfaceId id) -> void;
	auto ResizeSurface(SurfaceId id, u32 width, u32 height) -> void;
	auto GetSurfaceWidth(SurfaceId id) -> u32;
	auto GetSurfaceHeight(SurfaceId id) -> u32;
	auto RenderSurface(SurfaceId id) -> void;
}
