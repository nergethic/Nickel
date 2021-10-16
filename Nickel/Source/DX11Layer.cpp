#include "DX11Layer.h"

namespace Nickel::Renderer::DXLayer {
	// function inspired by: http://www.rastertek.com/dx11tut03.html
	auto QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync) -> DXGI_RATIONAL {
		DXGI_RATIONAL refreshRate = { 0, 1 };
		if (vsync) {
			IDXGIFactory2* factory;
			IDXGIAdapter* adapter;
			IDXGIOutput* adapterOutput;
			DXGI_MODE_DESC* displayModeList;

			HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&factory);
			if (FAILED(hr)) {
				MessageBox(0,
					TEXT("Could not create DXGIFactory instance."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				//throw new std::exception("Failed to create DXGIFactory.");
			}

			hr = factory->EnumAdapters(0, &adapter);
			if (FAILED(hr)) {
				MessageBox(0,
					TEXT("Failed to enumerate adapters."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				//throw new std::exception("Failed to enumerate adapters.");
			}

			hr = adapter->EnumOutputs(0, &adapterOutput);
			if (FAILED(hr)) {
				MessageBox(0,
					TEXT("Failed to enumerate adapter outputs."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				//throw new std::exception("Failed to enumerate adapter outputs.");
			}

			UINT numDisplayModes{};
			hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
			if (FAILED(hr)) {
				MessageBox(0,
					TEXT("Failed to query display mode list."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				//throw new std::exception("Failed to query display mode list.");
			}

			displayModeList = new DXGI_MODE_DESC[numDisplayModes];
			Assert(displayModeList);

			hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
			if (FAILED(hr)) {
				MessageBox(0,
					TEXT("Failed to query display mode list."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				//throw new std::exception("Failed to query display mode list.");
			}

			// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
			for (UINT i = 0; i < numDisplayModes; ++i) {
				if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight) {
					refreshRate = displayModeList[i].RefreshRate;
				}
			}

			delete[] displayModeList;
			SafeRelease(adapterOutput);
			SafeRelease(adapter);
			SafeRelease(factory);
		}

		return refreshRate;
	}

	auto GetHighestQualitySampleLevel(ID3D11Device1* device, DXGI_FORMAT format) -> UINT {
		UINT maxQualityLevelPlusOne;
		device->CheckMultisampleQualityLevels(format, 8, &maxQualityLevelPlusOne); // const_cast<ID3D11Device1&>(

		return maxQualityLevelPlusOne - 1;
	}

	auto CreateDevice() -> std::pair<ID3D11Device*, ID3D11DeviceContext*> {
		// Buffer Desc
		/*
		DXGI_MODE_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

		bufferDesc.Width = GlobalWindowWidth;
		bufferDesc.Height = GlobalWindowHeight;
		bufferDesc.RefreshRate.Numerator = 60;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		*/

		/*
		//Describe our SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = clientWidth;
		swapChainDesc.BufferDesc.Height = clientHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, false); // TODO vsync
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = rs.g_WindowHandle;
		swapChainDesc.SampleDesc.Count = MSAA_LEVEL;
		//swapChainDesc.SampleDesc.Quality = GetHighestQualitySampleLevel(device, DXGI_FORMAT format);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Windowed = TRUE;
		*/

		UINT deviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		if constexpr(_DEBUG) {
			deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
			// deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUGGABLE; // this shit just gave up and refuses to work (but be sure to have Graphics Tools feature installed on Win 10!)
		}

		D3D_FEATURE_LEVEL selectedFeatureLevel;

		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* deviceCtx = nullptr;
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, FEATURE_LEVELS, ArrayCount(FEATURE_LEVELS), D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &deviceCtx);
		if (result == E_INVALIDARG) { // Create device with feature levels up to 11_0
			result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, &FEATURE_LEVELS[1], ArrayCount(FEATURE_LEVELS) - 1, D3D11_SDK_VERSION, &device, &selectedFeatureLevel, &deviceCtx);
		}
		if (FAILED(result)) {
			//return -1;
			// TODO: Logger
		}

		return { device, deviceCtx };
	}

	auto CreateSwapChain(HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight) -> IDXGISwapChain1* {
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc1 = { 0 };

		swapChainDesc1.Stereo = FALSE;
		swapChainDesc1.BufferCount = 1; // 2;
		swapChainDesc1.Width = clientWidth;
		swapChainDesc1.Height = clientHeight;
		swapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc1.SampleDesc.Count = 4; // TODO use MSAA_LEVEL;
		swapChainDesc1.SampleDesc.Quality = GetHighestQualitySampleLevel(device, DXGI_FORMAT_R8G8B8A8_UNORM);
		swapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; // TODO: DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = { 0 };
		swapChainFullscreenDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, false);
		swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_CENTERED;
		swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainFullscreenDesc.Windowed = TRUE;

		IDXGIFactory2* pFactory;
		UINT factoryFlags = 0;
		if constexpr(_DEBUG) {
			factoryFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
		}
		HRESULT result = CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&pFactory));
		if (FAILED(result)) {
			//return -1; // TODO: logger
		}

		IDXGISwapChain1* swapChain1 = nullptr;
		ASSERT_ERROR_RESULT(pFactory->CreateSwapChainForHwnd(device, windowHandle, &swapChainDesc1, &swapChainFullscreenDesc, nullptr, &swapChain1));

		// SafeRelease(pFactory); // TODO
		return swapChain1;
	}

	auto CreateViewPort(f32 minX, f32 minY, f32 maxX, f32 maxY) -> D3D11_VIEWPORT {
		return {
			.TopLeftX = minX,
			.TopLeftY = minY,
			.Width = static_cast<FLOAT>(maxX),
			.Height = static_cast<FLOAT>(maxY),
			.MinDepth = D3D11_MIN_DEPTH,
			.MaxDepth = D3D11_MAX_DEPTH
		};
	}

	auto EnableDebug(const ID3D11Device1& device1, bool shouldBeVerbose) -> ID3D11Debug* {
		ID3D11Debug* d3dDebug{};

		auto d = const_cast<ID3D11Device1*>(&device1);
		auto result = d->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
		if (SUCCEEDED(result)) {
			ID3D11InfoQueue* d3dInfoQueue = nullptr;
			if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&d3dInfoQueue)))) {

				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_ERROR, true);
				if (shouldBeVerbose)
					d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_WARNING, true);

				D3D11_MESSAGE_ID hide[] = {
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
					// Add more message IDs here as needed
				};

				D3D11_INFO_QUEUE_FILTER filter;
				memset(&filter, 0, sizeof(filter));
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
				// d3dInfoQueue->Release();
			}

			// d3dDebug->Release();
		}

		return d3dDebug;
	}

	auto Clear(const CmdQueue& cmd, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void {
		cmd.queue->ClearRenderTargetView(renderTargetView, clearColor);
		cmd.queue->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
	}

	auto CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest) -> ID3D11DepthStencilState* {
		Assert(device != nullptr);
		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthStencilStateDesc.DepthEnable = enableDepthTest ? TRUE : FALSE;
		depthStencilStateDesc.DepthWriteMask = depthWriteMask;
		depthStencilStateDesc.DepthFunc = depthFunc;
		depthStencilStateDesc.StencilEnable = enableStencilTest ? TRUE : FALSE;

		ID3D11DepthStencilState* result = nullptr;
		HRESULT hr = device->CreateDepthStencilState(&depthStencilStateDesc, &result);
		if (!SUCCEEDED(hr)) {
			Assert(false);
			// TODO: report error
		}

		return result;
	}

	auto CreateDefaultRasterizerState(ID3D11Device1* device) -> ID3D11RasterizerState* {
		Assert(device != nullptr);

		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;

		ID3D11RasterizerState* result = nullptr;
		HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, &result);
		if (FAILED(hr)) {
			Assert(false);
			// TODO: report error
		}

		return result;
	}

	auto CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels = 1) -> ID3D11Texture2D* {
		Assert(device != nullptr);
		D3D11_TEXTURE2D_DESC depthStencilTextureDesc = { 0 };

		depthStencilTextureDesc.ArraySize = 1;
		depthStencilTextureDesc.BindFlags = bindFlags;
		depthStencilTextureDesc.CPUAccessFlags = 0; // No CPU access required.
		depthStencilTextureDesc.Format = format;
		depthStencilTextureDesc.Width = width;
		depthStencilTextureDesc.Height = height;
		depthStencilTextureDesc.MipLevels = mipLevels;
		depthStencilTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

		if (mipLevels == 1) {
			depthStencilTextureDesc.SampleDesc.Count = 4; // TODO: match depth stencil count and quality with swap chain settings
			depthStencilTextureDesc.SampleDesc.Quality = GetHighestQualitySampleLevel(device, format);
		}
		else {
			depthStencilTextureDesc.SampleDesc.Count = 1;
			depthStencilTextureDesc.SampleDesc.Quality = 0;
		}

		ID3D11Texture2D* depthStencilBuffer = nullptr;
		HRESULT hr = device->CreateTexture2D(&depthStencilTextureDesc, nullptr, &depthStencilBuffer);
		if (FAILED(hr)) {
			Assert(false);
			// TODO: report error
		}

		return depthStencilBuffer;
	}

	auto CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height) -> ID3D11Texture2D* {
		return CreateTexture(device, width, height, DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL);
	}

	auto CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture) -> ID3D11DepthStencilView* {
		Assert(device != nullptr);
		Assert(depthStencilTexture != nullptr);

		// D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		// ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		if (device->CreateDepthStencilView(depthStencilTexture, nullptr, nullptr) != S_FALSE) {
			// TODO: report error
			return nullptr;
		}

		ID3D11DepthStencilView* result = nullptr;
		device->CreateDepthStencilView(depthStencilTexture, nullptr, &result);
		if (FAILED(result)) {
			// TODO: report error
		}

		return result;
	}

	auto CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
		Assert(device != nullptr);

		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.Usage = usage;
		bufferDesc.BindFlags = bindFlags;
		bufferDesc.ByteWidth = byteWidthSize;
		bufferDesc.CPUAccessFlags = cpuAccessFlags; // D3D11_CPU_ACCESS_FLAG;
		bufferDesc.MiscFlags = miscFlags;           // D3D11_RESOURCE_MISC_FLAG
		// bufferDesc.StructureByteStride

		ID3D11Buffer* newBuffer = nullptr;
		HRESULT hr = device->CreateBuffer(&bufferDesc, initialData, &newBuffer);
		if (!SUCCEEDED(hr)) {
			// TODO: report error
			return nullptr;
		}

		return newBuffer;
	}

	auto Draw(const CmdQueue& cmd, int indexCount, int startVertex) -> void {
		if constexpr (_DEBUG)
			if (cmd.debug != nullptr) cmd.debug->ValidateContext(cmd.queue.Get());
		cmd.queue->Draw(indexCount, startVertex);
	}

	auto DrawIndexed(const CmdQueue& cmd, int indexCount, int startIndex, int startVertex) -> void {
		if constexpr(_DEBUG)
			if (cmd.debug != nullptr) cmd.debug->ValidateContext(cmd.queue.Get());
		cmd.queue->DrawIndexed(indexCount, startIndex, startVertex);
	}

	auto CreateInputLayout(ID3D11Device1* device, D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc, UINT vertexLayoutDescLength, const BYTE* shaderBytecodeWithInputSignature, SIZE_T shaderBytecodeSize) -> ID3D11InputLayout* {
		Assert(device != nullptr);
		Assert(vertexLayoutDesc != nullptr);
		Assert(shaderBytecodeWithInputSignature != nullptr);

		ID3D11InputLayout* result = nullptr;

		HRESULT hr = device->CreateInputLayout(
			vertexLayoutDesc,
			vertexLayoutDescLength,
			shaderBytecodeWithInputSignature,
			shaderBytecodeSize,
			&result);

		if (FAILED(hr)) {
			// TODO: log error
			Assert(nullptr);
		}

		return result;
	}

	auto CreateVertexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
		Assert(device != nullptr);

		auto newVertexBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, size, 0, 0, initialData);
		//i32 newVertexBufferIndex = rs->vertexBuffersCount; // TODO: if vertex buffer creation fails this still returns index to empty array 
		//if (newVertexBuffer != nullptr) {
			//rs->vertexBuffers[newVertexBufferIndex] = newVertexBuffer;
			//rs->vertexBuffersCount++;
		//}

		return newVertexBuffer;
	}

	auto CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
		Assert(device != nullptr);
		auto newIndexBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER, size, 0, 0, initialData);

		return newIndexBuffer;
	}

	auto CreateConstantBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData) -> ID3D11Buffer* {
		Assert(device != nullptr);
		auto newConstantBuffer = CreateBuffer(device, D3D11_USAGE::D3D11_USAGE_DEFAULT, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, size, 0, 0, initialData);

		return newConstantBuffer;
	}

	auto GetHResultString(HRESULT errCode) -> std::string {
		switch (errCode) {
			// Windows
		case S_OK:           return "S_OK";
		case E_NOTIMPL:      return "E_NOTIMPL";
		case E_NOINTERFACE:  return "E_NOINTERFACE";
		case E_POINTER:      return "E_POINTER";
		case E_ABORT:        return "E_ABORT";
		case E_FAIL:         return "E_FAIL";
		case E_UNEXPECTED:   return "E_UNEXPECTED";
		case E_ACCESSDENIED: return "E_ACCESSDENIED";
		case E_HANDLE:       return "E_HANDLE";
		case E_OUTOFMEMORY:  return "E_OUTOFMEMORY";
		case E_INVALIDARG:   return "E_INVALIDARG";

			// DX11
		case D3D11_ERROR_FILE_NOT_FOUND:                return "D3D11_ERROR_FILE_NOT_FOUND";
		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";

			// DXGI
		case DXGI_ERROR_ACCESS_DENIED:                return "DXGI_ERROR_ACCESS_DENIED";
		case DXGI_ERROR_ACCESS_LOST:                  return "DXGI_ERROR_ACCESS_LOST";
		case DXGI_ERROR_ALREADY_EXISTS:               return "DXGI_ERROR_ALREADY_EXISTS";
		case DXGI_ERROR_CANNOT_PROTECT_CONTENT:       return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";
		case DXGI_ERROR_DEVICE_HUNG:                  return "DXGI_ERROR_DEVICE_HUNG";
		case DXGI_ERROR_DEVICE_REMOVED:               return "DXGI_ERROR_DEVICE_REMOVED";
		case DXGI_ERROR_DEVICE_RESET:                 return "DXGI_ERROR_DEVICE_RESET";
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR:        return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
		case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:    return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
		case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
		case DXGI_ERROR_INVALID_CALL:                 return "DXGI_ERROR_INVALID_CALL";
		case DXGI_ERROR_MORE_DATA:                    return "DXGI_ERROR_MORE_DATA";
		case DXGI_ERROR_NAME_ALREADY_EXISTS:          return "DXGI_ERROR_NAME_ALREADY_EXISTS";
		case DXGI_ERROR_NONEXCLUSIVE:                 return "DXGI_ERROR_NONEXCLUSIVE";
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:      return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
		case DXGI_ERROR_NOT_FOUND:                    return "DXGI_ERROR_NOT_FOUND";
		case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:   return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
		case DXGI_ERROR_REMOTE_OUTOFMEMORY:           return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
		case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:     return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";
		case DXGI_ERROR_SDK_COMPONENT_MISSING:        return "DXGI_ERROR_SDK_COMPONENT_MISSING";
		case DXGI_ERROR_SESSION_DISCONNECTED:         return "DXGI_ERROR_SESSION_DISCONNECTED";
		case DXGI_ERROR_UNSUPPORTED:                  return "DXGI_ERROR_UNSUPPORTED";
		case DXGI_ERROR_WAIT_TIMEOUT:                 return "DXGI_ERROR_WAIT_TIMEOUT";
		case DXGI_ERROR_WAS_STILL_DRAWING:            return "DXGI_ERROR_WAS_STILL_DRAWING";

		default: return "Unhandled HRESULT code: " + std::to_string(static_cast<u32>(errCode));
		}

		return "";
	}

	auto GetHResultErrorMessage(HRESULT errCode) -> std::string {
		std::string result = GetHResultString(errCode);
		// TODO: add additional checks for robustness
		// if DXGI_ERROR_DEVICE_REMOVED, DXGI_ERROR_DEVICE_RESET or DXGI_ERROR_DRIVER_INTERNAL_ERROR 
		// ID3D11Device device - GetDeviceRemovedReason();

		return result;
	}
}
