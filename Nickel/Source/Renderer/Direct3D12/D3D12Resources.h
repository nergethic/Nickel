#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer {
	class DescriptorHeap;

	struct DescriptorHandle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
		u32 index = u32_invalid_id;

		constexpr auto IsValid() const -> bool { return cpu.ptr != 0; }
		constexpr auto IsShaderVisible() const -> bool { return gpu.ptr != 0; }

	private:
#ifdef _DEBUG
		friend class DescriptorHeap;
		DescriptorHeap* container = nullptr;
#endif
	};

	class DescriptorHeap {
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : type(type) {}

		DescriptorHeap() = default;
		explicit DescriptorHeap(DescriptorHeap& o) = delete;
		explicit DescriptorHeap(DescriptorHeap&& o) = delete;
		auto operator=(const DescriptorHeap&)->DescriptorHeap & = delete;
		auto operator=(const DescriptorHeap&&)->DescriptorHeap & = delete;

		auto Initialize(u32 capacity, bool isShaderVisible) -> bool;
		auto ProcessDeferredFree(u32 frameIndex) -> void;
		auto Release() -> void;

		[[nodiscard]] auto Allocate()->DescriptorHandle;
		auto Free(DescriptorHandle& handle) -> void;

		constexpr auto GetType()      const -> D3D12_DESCRIPTOR_HEAP_TYPE { return type; };
		constexpr auto GetCpuStartAddressHandle() const -> D3D12_CPU_DESCRIPTOR_HANDLE { return cpuStartAddressHandle; };
		constexpr auto GetGpuStartAddressHandle() const -> D3D12_GPU_DESCRIPTOR_HANDLE { return gpuStartAddressHandle; };
		constexpr auto GetHeap()      const -> ID3D12DescriptorHeap* const { return heap; };

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
		std::vector<u32> deferredFreeIndices[3]{};
		std::mutex mutex;
		u32 capacity = 0;
		u32 size = 0;
		u32 descriptorSize = 0;
		const D3D12_DESCRIPTOR_HEAP_TYPE type;
	};

	struct D3D12TextureInitData {
		ID3D12Heap1* heap;
		ID3D12Resource* resource = nullptr;
		D3D12_RESOURCE_ALLOCATION_INFO1 allocationInfo;
		D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr;
		D3D12_RESOURCE_DESC* desc = nullptr;
		D3D12_RESOURCE_STATES initialState{};
		D3D12_CLEAR_VALUE clearValue;
	};

	class D3D12Texture {
	public:
		constexpr static u32 MAX_MIPS = 14;
		D3D12Texture() = default;
		explicit D3D12Texture(D3D12TextureInitData data);
		explicit D3D12Texture(const D3D12Texture&) = delete;
		auto operator=(const D3D12Texture&)->D3D12Texture & = delete;
		constexpr D3D12Texture(D3D12Texture&& o) : resource(o.resource), srv(srv) {
			o.Reset();
		}

		constexpr auto operator=(D3D12Texture&& o) -> D3D12Texture& {
			Assert(this != &o);
			if (this != &o) {
				Release();
				Move(o);
			}
			return *this;
		}

		~D3D12Texture() { Release(); }

		auto Release() -> void;
		constexpr auto GetResource() const -> ID3D12Resource* const { return resource; }
		constexpr auto GetSrv() const -> DescriptorHandle const { return srv; }
	private:
		constexpr auto Move(D3D12Texture& o) -> void {
			resource = o.resource;
			srv = o.srv;
			o.Reset();
		}

		constexpr auto Reset() -> void {
			resource = nullptr;
			srv = {};
		}

		ID3D12Resource* resource{ nullptr };
		DescriptorHandle srv;
	};

	class D3D12RenderTexture {
	public:
		D3D12RenderTexture() = default;
		explicit D3D12RenderTexture(D3D12TextureInitData data);
		explicit D3D12RenderTexture(const D3D12RenderTexture&) = delete;
		auto operator=(const D3D12RenderTexture&) -> D3D12RenderTexture& = delete;
		constexpr D3D12RenderTexture(D3D12RenderTexture&& o)
			: texture{ std::move(o.texture) }, mipCount{ o.mipCount }
		{
			for (u32 i{ 0 }; i < mipCount; ++i)
				rtv[i] = o.rtv[i];
			o.Reset();
		}

		constexpr auto operator=(D3D12RenderTexture&& o) -> D3D12RenderTexture& {
			Assert(this != &o);
			if (this != &o) {
				Release();
				Move(o);
			}
			return *this;
		}

		~D3D12RenderTexture() { Release(); }

		void Release();
		constexpr auto GetMipCount() const -> u32 { return mipCount; }
		constexpr auto GetRtv(u32 mipIndex) const -> D3D12_CPU_DESCRIPTOR_HANDLE { Assert(mipIndex < mipCount); return rtv[mipIndex].cpu; }
		constexpr auto GetSrv() const -> DescriptorHandle { return texture.GetSrv(); }
		constexpr auto GetResource() const -> ID3D12Resource* const { return texture.GetResource(); }
	private:
		constexpr auto Move(D3D12RenderTexture& o) -> void {
			texture = std::move(o.texture);
			mipCount = o.mipCount;
			for (u32 i{ 0 }; i < mipCount; ++i)
				rtv[i] = o.rtv[i];
			o.Reset();
		}

		constexpr auto Reset() -> void {
			for (u32 i{ 0 }; i < mipCount; ++i)
				rtv[i] = {};
			mipCount = 0;
		}

		D3D12Texture       texture{};
		DescriptorHandle   rtv[D3D12Texture::MAX_MIPS]{};
		u32                mipCount{ 0 };
	};

	class D3D12DepthBuffer {
	public:
		D3D12DepthBuffer() = default;
		explicit D3D12DepthBuffer(D3D12TextureInitData data);
		explicit D3D12DepthBuffer(const D3D12RenderTexture&) = delete;
		auto operator=(const D3D12DepthBuffer&) -> D3D12DepthBuffer& = delete;

		constexpr D3D12DepthBuffer(D3D12DepthBuffer&& o)
			: texture{ std::move(o.texture) }, dsv{ o.dsv }
		{
			o.dsv = {};
		}

		constexpr auto operator=(D3D12DepthBuffer&& o) -> D3D12DepthBuffer& {
			Assert(this != &o);
			if (this != &o) {
				texture = std::move(o.texture);
				dsv = o.dsv;
				o.dsv = {};
			}
			return *this;
		}

		~D3D12DepthBuffer() { Release(); }

		void Release();
		constexpr auto GetDsv() const -> D3D12_CPU_DESCRIPTOR_HANDLE { return dsv.cpu; }
		constexpr auto GetSrv() const -> DescriptorHandle { return texture.GetSrv(); }
		constexpr auto GetResource() const ->ID3D12Resource* const { return texture.GetResource(); }

	private:
		D3D12Texture     texture{};
		DescriptorHandle dsv{};
	};
}