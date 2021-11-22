#include "Resources.h"
#include "D3D12Core.h"

namespace Nickel::Renderer::DX12Layer {
	auto DescriptorHeap::Initialize(u32 _capacity, bool isShaderVisible) -> bool {
		std::lock_guard lock{mutex};
		Assert(_capacity != 0 && _capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		Assert(!(type == D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER &&
			_capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));

		if (type == D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
			type == D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			isShaderVisible = false;
		}

		Release();

		ID3D12Device* device = Core::GetDevice();
		Assert(device != nullptr);

		D3D12_DESCRIPTOR_HEAP_DESC desc{
			.Type = type,
			.NumDescriptors = _capacity,
			.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};
		ASSERT_ERROR_RESULT(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap)));

		freeHandles = std::move(std::make_unique<u32[]>(_capacity));
		capacity = _capacity;
		size = 0;

		for (u32 i = 0; i < capacity; i++)
			freeHandles[i] = i;

		descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		cpuStartAddressHandle = heap->GetCPUDescriptorHandleForHeapStart();
		gpuStartAddressHandle = isShaderVisible ? heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{0};

		return true;
	}

	auto DescriptorHeap::Release() -> void {

	}

	[[nodiscard]] auto DescriptorHeap::Allocate() -> DescriptorHandle {
		std::lock_guard lock{ mutex };
		Assert(heap != nullptr);
		Assert(size < capacity);

		ID3D12Device* device = Core::GetDevice();
		Assert(device != nullptr);

		const u32 index = freeHandles[size];
		const u32 absoluteOffset = index * descriptorSize;
		size++;

		DescriptorHandle handle;
		handle.cpu.ptr = cpuStartAddressHandle.ptr + absoluteOffset;
		if (IsShaderVisible())
			handle.gpu.ptr = gpuStartAddressHandle.ptr + absoluteOffset;

		if constexpr (_DEBUG) {
			handle.container = this;
			handle.index = index;
		}

		return handle;
	}

	auto DescriptorHeap::Free(DescriptorHandle& handle) -> void {
		if (!handle.IsValid())
			return;

		std::lock_guard lock{ mutex };
		Assert(heap != nullptr && size != 0);
		Assert(handle.container = this);
		Assert(handle.cpu.ptr >= cpuStartAddressHandle.ptr);
		Assert((handle.cpu.ptr - cpuStartAddressHandle.ptr) % descriptorSize == 0);
		Assert(handle.index < capacity);
		const u32 index = static_cast<u32>(handle.cpu.ptr - cpuStartAddressHandle.ptr) / descriptorSize;
		Assert(handle.index == index);

		// TODO: implement defered free system, other threads could be using a heap so we need to wait until they are done with it and then free
		handle = {};
	}
}