#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer {
	class DescriptorHeap;

	struct DescriptorHandle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};

		constexpr auto IsValid() const -> bool { return cpu.ptr != 0; }
		constexpr auto IsShaderVisible() const -> bool { return gpu.ptr != 0; }

	private:
#ifdef _DEBUG
		friend class DescriptorHeap;
		DescriptorHeap* container = nullptr;
		u32 index = 0;
#endif
	};

	class DescriptorHeap {
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : type(type) {}

		DescriptorHeap() = default;
		explicit DescriptorHeap(DescriptorHeap& o) = delete;
		explicit DescriptorHeap(DescriptorHeap&& o) = delete;
		auto operator=(const DescriptorHeap&) -> DescriptorHeap & = delete;
		auto operator=(const DescriptorHeap&&) -> DescriptorHeap & = delete;

		auto Initialize(u32 capacity, bool isShaderVisible) -> bool;
		auto Release() -> void;

		[[nodiscard]] auto Allocate() -> DescriptorHandle;
		auto Free(DescriptorHandle& handle) -> void;

		constexpr auto GetType()      const -> D3D12_DESCRIPTOR_HEAP_TYPE { return type; };
		constexpr auto GetCpuStartAddressHandle() const -> D3D12_CPU_DESCRIPTOR_HANDLE { return cpuStartAddressHandle; };
		constexpr auto GetGpuStartAddressHandle() const -> D3D12_GPU_DESCRIPTOR_HANDLE { return gpuStartAddressHandle; };
		constexpr auto GetHeap()      const -> ID3D12DescriptorHeap *const { return heap; };

		constexpr auto GetCapacity()       const -> u32 { return capacity; };
		constexpr auto GetSize()           const -> u32 { return size; };
		constexpr auto GetDescriptorSize() const -> u32 { return descriptorSize; };
		constexpr auto IsShaderVisible()   const -> bool { return gpuStartAddressHandle.ptr != 0;; };

		~DescriptorHeap() {
			Assert(heap == nullptr);
		}
	private:
		ID3D12DescriptorHeap* heap;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuStartAddressHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStartAddressHandle{};
		std::unique_ptr<u32[]> freeHandles{};
		std::mutex mutex;
		u32 capacity = 0;
		u32 size = 0;
		u32 descriptorSize = 0;
		const D3D12_DESCRIPTOR_HEAP_TYPE type;
	};
}