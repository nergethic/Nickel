#include "D3D12Core.h"
#include "Resources.h"

using namespace Microsoft::WRL;

namespace Nickel::Renderer::DX12Layer::Core {
namespace {
	class D3D12Command {
	public:
		constexpr auto GetCmdQueue() const -> ID3D12CommandQueue* const { return cmdQueue; }
		constexpr auto GetCmdList() -> ID3D12GraphicsCommandList6* { return cmdList; }
		constexpr auto GetCurrentFrameIndex() const -> u32{ return currentFrameIndex; }

		D3D12Command() = default;
		explicit D3D12Command(D3D12Command& o) = delete;
		explicit D3D12Command(D3D12Command&& o) = delete;
		auto operator=(const D3D12Command&) -> D3D12Command& = delete;
		auto operator=(const D3D12Command&&) -> D3D12Command& = delete;

		explicit D3D12Command(ID3D12Device8& device, D3D12_COMMAND_LIST_TYPE type) {
			D3D12_COMMAND_QUEUE_DESC desc{
				.Type = type,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0,
			};

			device.CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue));
			cmdQueue->SetName(type == D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT ?
				L"GFX cmd queue" :
				type == D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE ?
				L"compute cmd queue" : L"cmd queue");

			for (u32 i = 0; i < ArrayCount(cmdFrames); i++) {
				CommandFrame& frame = cmdFrames[i];
				ASSERT_ERROR_RESULT(device.CreateCommandAllocator(type, IID_PPV_ARGS(&frame.cmdAllocator)));
				frame.cmdAllocator->SetName(L"cmd allocator " + i);
			}

			ASSERT_ERROR_RESULT(device.CreateCommandList(0, type, cmdFrames[0].cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList)));
			ASSERT_ERROR_RESULT(cmdList->Close());
			cmdList->SetName(type == D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT ?
				L"GFX cmd list" :
				type == D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE ?
				L"compute cmd queue" : L"cmd list");

			ASSERT_ERROR_RESULT(device.CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
			fence->SetName(L"Fence");
			fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
			Assert(fenceEvent);
		}

		~D3D12Command() {
			Assert(cmdQueue == nullptr);
			Assert(cmdList == nullptr);
			Assert(fence == nullptr);
		}

		auto BeginFrame() -> void {
			auto& cmdFrame = GetCurrentFrame();
			cmdFrame.Wait(fenceEvent, fence);
			ASSERT_ERROR_RESULT(cmdFrame.cmdAllocator->Reset()); // frees memory used by previously recorded commands
			cmdList->Reset(cmdFrame.cmdAllocator, nullptr);
		}

		auto EndFrame() -> void {
			ASSERT_ERROR_RESULT(cmdList->Close());
			ID3D12CommandList* const cmdLists[]{ cmdList };
			cmdQueue->ExecuteCommandLists(ArrayCount(cmdLists), cmdLists);

			fenceValue++;
			auto& cmdFrame = GetCurrentFrame();
			cmdFrame.fenceValue = fenceValue;
			cmdQueue->Signal(fence, fenceValue);

			currentFrameIndex = (currentFrameIndex + 1) % ArrayCount(cmdFrames);
		}

		auto Flush() -> void {
			for (int i = 0; i < ArrayCount(cmdFrames); i++)
				cmdFrames[i].Wait(fenceEvent, fence);
			currentFrameIndex = 0;
		}

		auto Release() -> void {
			Flush();
			SafeRelease(fence);
			fenceValue = 0;

			CloseHandle(fenceEvent);
			fenceEvent = nullptr;

			SafeRelease(cmdQueue);
			SafeRelease(cmdList);

			for (int i = 0; i < ArrayCount(cmdFrames); i++)
				cmdFrames[i].Release();
		}

	private:
		struct CommandFrame {
			ID3D12CommandAllocator* cmdAllocator = nullptr;
			u64 fenceValue = 0;

			auto Wait(HANDLE fenceEvent, ID3D12Fence* fence) -> void {
				Assert(fenceEvent && fence != nullptr);
				if (fence->GetCompletedValue() < fenceValue) {
					fence->SetEventOnCompletion(fenceValue, fenceEvent);
					WaitForSingleObject(fenceEvent, INFINITE);
				}
			}

			auto Release() -> void {
				SafeRelease(cmdAllocator);
				fenceValue = 0;
			}
		};

		auto GetCurrentFrame() -> CommandFrame& {
			return cmdFrames[currentFrameIndex];
		}

		ID3D12CommandQueue* cmdQueue = nullptr;
		ID3D12GraphicsCommandList6* cmdList = nullptr;
		CommandFrame cmdFrames[3] = {};
		u32 currentFrameIndex = 0;
		ID3D12Fence* fence = nullptr;
		HANDLE fenceEvent = nullptr;
		u64 fenceValue = 0;
	};

	ID3D12Device8* mainDevice = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;
	D3D12Command cmd;
	u32 deferredReleasesFlag[3]{};
	std::mutex deferredReleasesMutex{};
	std::vector<IUnknown*> deferredReleases[3]{}; // TODO: change to utility vector
	DescriptorHeap rtvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
	DescriptorHeap dsvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
	DescriptorHeap srvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
	DescriptorHeap uavDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	constexpr D3D_FEATURE_LEVEL targetRequiredFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1; // NOTE: we need mesh shaders support

	auto GetMainAdapter() -> IDXGIAdapter4* {
		IDXGIAdapter4* adapter = nullptr;
		for (u32 i = 0; i < dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++) {
			if (SUCCEEDED(D3D12CreateDevice(adapter, targetRequiredFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
				return adapter;
			}
			SafeRelease(adapter);
		}
		
		return nullptr;
	}
}

auto __declspec(noinline) ProcessDeferredReleases(u32 frameIndex) -> void {
	std::lock_guard lock{ deferredReleasesMutex };
	deferredReleasesFlag[frameIndex] = 0;

	rtvDescHeap.ProcessDeferredFree(frameIndex);
	dsvDescHeap.ProcessDeferredFree(frameIndex);
	srvDescHeap.ProcessDeferredFree(frameIndex);
	uavDescHeap.ProcessDeferredFree(frameIndex);

	std::vector<IUnknown*>& resources{ deferredReleases[frameIndex] }; // TODO: change to utility
	if (!resources.empty()) {
		for (auto& r : resources)
			SafeRelease(r);
		resources.clear();
	}
}

namespace Internal {
	auto DeferredRelease(IUnknown* ptr) -> void {
		const u32 frameIndex = GetCurrentFrameIndex();
		std::lock_guard lock{ deferredReleasesMutex };
		deferredReleases[frameIndex].push_back(ptr);
		SetDeferredReleasesFlag();
	}
}

auto Init() -> bool {
	if (mainDevice != nullptr)
		Shutdown();

	u32 dxgiFactoryFlags = 0;
	if constexpr (_DEBUG) {
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		ComPtr<ID3D12Debug3> debugInterface;
		if SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))) {
			debugInterface->EnableDebugLayer();
			debugInterface->SetEnableGPUBasedValidation(TRUE); // NOTE: this makes things run super slow 
			debugInterface->Release();
		} else
			Logger::Error("Debug interface cannot be created. Check if Graphics Tools option is enabled in this system");
	}

	ASSERT_ERROR_RESULT(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	ComPtr<IDXGIAdapter4> mainAdapter;
	mainAdapter.Attach(GetMainAdapter());
	if (mainAdapter == nullptr) {
		Logger::Error("Couldn't find appriopriate adapter");
		return false;
	}

	ASSERT_ERROR_RESULT(D3D12CreateDevice(mainAdapter.Get(), targetRequiredFeatureLevel, IID_PPV_ARGS(&mainDevice)));
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 featureData{};
	mainDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &featureData, sizeof(featureData));
	if (featureData.MeshShaderTier == 0) { // D3D12DDI_MESH_SHADER_TIER_NOT_SUPPORTED
		Logger::Error("Mesh shaders aren't supported on created device");
		return false;
	}
		

	mainDevice->SetName(L"Main DX12 device");

	if constexpr (_DEBUG) {
		ID3D12InfoQueue* infoQueue;
		ASSERT_ERROR_RESULT(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->Release();
	}

	bool heapInitResult = true;
	heapInitResult &= rtvDescHeap.Initialize(512, false);
	heapInitResult &= dsvDescHeap.Initialize(512, false);
	heapInitResult &= srvDescHeap.Initialize(4096, true);
	heapInitResult &= uavDescHeap.Initialize(512, true);
	if (!heapInitResult)
		return false;

	rtvDescHeap.GetHeap()->SetName(L"RTV Descriptor Heap");
	dsvDescHeap.GetHeap()->SetName(L"DSV Descriptor Heap");
	srvDescHeap.GetHeap()->SetName(L"SRV Descriptor Heap");
	uavDescHeap.GetHeap()->SetName(L"UAV Descriptor Heap");

	new (&cmd) D3D12Command(*mainDevice, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (cmd.GetCmdQueue() == nullptr)
		return false;

	return true;
}

auto Shutdown() -> void {
	cmd.Release();

	for (u32 i = 0; i < 3; i++)
		ProcessDeferredReleases(i);

	rtvDescHeap.Release();
	dsvDescHeap.Release();
	srvDescHeap.Release();
	uavDescHeap.Release();

	ProcessDeferredReleases(0);
	
	if constexpr (_DEBUG) {
		ID3D12InfoQueue* infoQueue;
		ASSERT_ERROR_RESULT(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, false);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, false);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
		infoQueue->Release();

		ComPtr<ID3D12DebugDevice2> debugDevice;
		ASSERT_ERROR_RESULT(mainDevice->QueryInterface(IID_PPV_ARGS(&debugDevice)));
		SafeRelease(mainDevice);
		ASSERT_ERROR_RESULT(debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_FLAGS::D3D12_RLDO_SUMMARY |
			D3D12_RLDO_FLAGS::D3D12_RLDO_DETAIL |
			D3D12_RLDO_FLAGS::D3D12_RLDO_IGNORE_INTERNAL));
	}
	
	SafeRelease(mainDevice);
	SafeRelease(dxgiFactory);
}

auto Render() -> void {
	cmd.BeginFrame();
	ID3D12GraphicsCommandList6* cmdList = cmd.GetCmdList();

	const u32 frameIndex = GetCurrentFrameIndex();
	if (deferredReleasesFlag[frameIndex]) {
		ProcessDeferredReleases(frameIndex);
	}
	cmd.EndFrame();
}

auto GetDevice()->ID3D12Device* const {
	return mainDevice;
}

auto GetCurrentFrameIndex() -> u32 {
	return cmd.GetCurrentFrameIndex();
}

auto SetDeferredReleasesFlag() -> void {
	deferredReleasesFlag[GetCurrentFrameIndex()] = 1;
}
}