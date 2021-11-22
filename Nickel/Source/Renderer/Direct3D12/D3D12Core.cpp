#include "D3D12Core.h"

using namespace Microsoft::WRL;

namespace Nickel::Renderer::DX12Layer::Core {
namespace {
	ID3D12Device8* mainDevice = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;

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

auto Init() -> bool {
	if (mainDevice != nullptr)
		Shutdown();

	u32 dxgiFactoryFlags = 0;
	if constexpr (_DEBUG) {
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		ComPtr<ID3D12Debug3> debugInterface;
		ASSERT_ERROR_RESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
		debugInterface->SetEnableGPUBasedValidation(TRUE); // NOTE: this makes things run super slow 
		debugInterface->Release();
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
		ComPtr<ID3D12InfoQueue> infoQueue;
		ASSERT_ERROR_RESULT(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->Release();
	}

	return true;
}

auto Shutdown() -> void {
	if constexpr (_DEBUG) {
		ComPtr<ID3D12InfoQueue> infoQueue;
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
}