#include "D3D12Resources.h"
#include "D3D12Core.h"
#include "D3D12Helpers.h"

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
		for (u32 i = 0; i < 3; i++) {
			Assert(deferredFreeIndices[i].empty());
		}

		descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		cpuStartAddressHandle = heap->GetCPUDescriptorHandleForHeapStart();
		gpuStartAddressHandle = isShaderVisible ? heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{0};

		return true;
	}

	auto DescriptorHeap::Release() -> void {
		Assert(size == 0);
		Core::SafeDeferredRelease(heap);
	}

	auto DescriptorHeap::ProcessDeferredFree(u32 frameIndex) -> void {
		std::lock_guard lock{ mutex };
		Assert(frameIndex < 3);

		std::vector<u32>& indices{ deferredFreeIndices[frameIndex] }; // TODO: change to custom vector
		if (!indices.empty()) {
			for (auto index : indices) {
				size--;
				freeHandles[size] = index;
			}
			indices.clear();
		}
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

		const u32 frameIndex = Core::GetCurrentFrameIndex();
		deferredFreeIndices[frameIndex].push_back(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}

	// --- D3D12Texture ---
	D3D12Texture::D3D12Texture(D3D12TextureInitData data) {
		auto* const device{ Core::GetDevice() };
		Assert(device != nullptr);

		D3D12_CLEAR_VALUE* const clearVal = (data.desc && (data.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
			data.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			? &data.clearValue : nullptr;
		if (data.resource) {
			Assert(data.heap == nullptr);
			resource = data.resource;
		}
		else if (data.heap != nullptr && data.desc != nullptr) {
			Assert(data.resource == nullptr);
			ASSERT_ERROR_RESULT(device->CreatePlacedResource(data.heap, data.allocationInfo.Offset, data.desc, data.initialState, clearVal, IID_PPV_ARGS(&resource)));
		} else if (data.desc != nullptr) {
			Assert(data.resource != nullptr && data.heap == nullptr);
			const auto& defaultHeap = primal::graphics::d3d12::d3dx::heapProperties.defaultHeap;
			ASSERT_ERROR_RESULT(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, data.desc, data.initialState, clearVal, IID_PPV_ARGS(&resource)));
		}

		Assert(resource != nullptr);
		srv = Core::GetSrvHeap().Allocate();
		device->CreateShaderResourceView(resource, data.srvDesc, srv.cpu);
	}

	auto D3D12Texture::Release() -> void {
		Core::GetSrvHeap().Free(srv);
		Core::SafeDeferredRelease(resource);
	}

	// --- D3D12RenderTexture ---
	D3D12RenderTexture::D3D12RenderTexture(D3D12TextureInitData data)
		: texture{ data }
	{
		Assert(data.desc);
		mipCount = GetResource()->GetDesc().MipLevels;
		Assert(mipCount && mipCount <= D3D12Texture::MAX_MIPS);

		DescriptorHeap& rtvHeap{ Core::GetRtvHeap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = data.desc->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		auto* const device{ Core::GetDevice() };
		assert(device);

		for (u32 i = 0; i < mipCount; ++i) {
			rtv[i] = rtvHeap.Allocate();
			device->CreateRenderTargetView(GetResource(), &desc, rtv[i].cpu);
			++desc.Texture2D.MipSlice;
		}
	}

	void D3D12RenderTexture::Release() {
		for (u32 i{ 0 }; i < mipCount; ++i)
			Core::GetRtvHeap().Free(rtv[i]);
		texture.Release();
		mipCount = 0;
	}

	// --- D3D12DepthBuffer ---
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12TextureInitData data) {
		Assert(data.desc);
		const DXGI_FORMAT dsvFormat{ data.desc->Format };

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		if (data.desc->Format == DXGI_FORMAT_D32_FLOAT) {
			data.desc->Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		Assert(!data.srvDesc && !data.resource);
		data.srvDesc = &srvDesc;
		texture = D3D12Texture(data);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Format = dsvFormat;
		dsvDesc.Texture2D.MipSlice = 0;

		dsv = Core::GetDsvHeap().Allocate();

		auto* const device{ Core::GetDevice() };
		assert(device);
		device->CreateDepthStencilView(GetResource(), &dsvDesc, dsv.cpu);
	}

	void D3D12DepthBuffer::Release() {
		Core::GetDsvHeap().Free(dsv);
		texture.Release();
	}
}