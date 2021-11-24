#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer {
	class DescriptorHeap;
}

namespace Nickel::Renderer::DX12Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;
	auto Render() -> void;

	auto GetDevice() -> ID3D12Device *const;
	auto GetCurrentFrameIndex() -> u32;
	auto SetDeferredReleasesFlag() -> void;
	auto GetDefaultRenderTargetFormat() -> DXGI_FORMAT;
	auto GetRtvHeap() -> DescriptorHeap&;
	auto GetDsvHeap() -> DescriptorHeap&;
	auto GetSrvHeap() -> DescriptorHeap&;
	auto GetUavHeap() -> DescriptorHeap&;

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
}