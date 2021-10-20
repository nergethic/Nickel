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
			Logger::Error("D3D11Device creation failed");
			Assert(false);
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
			Logger::Error("DXGIFactory2 creation failed");
			Assert(false);
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

	auto Clear(const CmdQueue& cmd, u32 clearFlag, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void {
		const bool shouldClearColor   = clearFlag & static_cast<u32>(ClearFlag::CLEAR_COLOR);
		const bool shouldClearDepth   = clearFlag & static_cast<u32>(ClearFlag::CLEAR_DEPTH);
		const bool shouldClearStencil = clearFlag & static_cast<u32>(ClearFlag::CLEAR_STENCIL);

		if (shouldClearColor) {
			if (renderTargetView != nullptr)
				cmd.queue->ClearRenderTargetView(renderTargetView, clearColor);
			else
				Logger::Error("clear flag has CLEAR_COLOR bit but passed renderTargetView is null");
		}

		if (shouldClearDepth || shouldClearStencil) {
			const u32 depthStencilClearFlag = (shouldClearDepth ? D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH : 0) | (shouldClearStencil ? D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL : 0);
			if (depthStencilView != nullptr)
				cmd.queue->ClearDepthStencilView(depthStencilView, depthStencilClearFlag, clearDepth, clearStencil);
			else
				Logger::Error("clear flag has CLEAR_DEPTH or CLEAR_STENCIL bit but passed depthStencilView is null");
		}
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
			Logger::Error("Depth Stencil State creation failed");
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

	auto CreateSamplerState(ID3D11Device* device, const D3D11_SAMPLER_DESC& desc) -> ID3D11SamplerState* {
		ID3D11SamplerState* result{};
		ASSERT_ERROR_RESULT(device->CreateSamplerState(&desc, &result));
		return result;
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
			if (cmd.debug != nullptr)
				cmd.debug->ValidateContext(cmd.queue.Get());
		cmd.queue->Draw(indexCount, startVertex);
	}

	auto DrawIndexed(const CmdQueue& cmd, int indexCount, int startIndex, int startVertex) -> void {
		if constexpr(_DEBUG)
			if (cmd.debug != nullptr)
				cmd.debug->ValidateContext(cmd.queue.Get());
		cmd.queue->DrawIndexed(indexCount, startIndex, startVertex);
	}

	auto CreateInputLayout(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> shaderBytecodeWithInputSignature) -> ID3D11InputLayout* {
		Assert(device != nullptr);
		Assert(vertexLayoutDesc.size() > 0 && vertexLayoutDesc.data() != nullptr);
		Assert(shaderBytecodeWithInputSignature.size() > 0 && shaderBytecodeWithInputSignature.data() != nullptr);

		ID3D11InputLayout* result = nullptr;

		if constexpr (_DEBUG) { // NOTE: this call validates other input parameters
			const HRESULT validationResult = device->CreateInputLayout(
				vertexLayoutDesc.data(),
				vertexLayoutDesc.size(),
				shaderBytecodeWithInputSignature.data(),
				shaderBytecodeWithInputSignature.size(),
				nullptr);
			Assert(validationResult == S_FALSE);
		}

		ASSERT_ERROR_RESULT(device->CreateInputLayout(
			vertexLayoutDesc.data(),
			vertexLayoutDesc.size(),
			shaderBytecodeWithInputSignature.data(),
			shaderBytecodeWithInputSignature.size(),
			&result));

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

	auto SetVertexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* vertexBuffer, UINT stride, UINT offset) -> void {
		NoConst(cmdQueue).IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	auto SetIndexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* indexBuffer, DXGI_FORMAT format, u32 offset) -> void {
		NoConst(cmdQueue).IASetIndexBuffer(indexBuffer, format, offset);
	}

	auto DrawIndexed(const ID3D11DeviceContext1& cmdQueue, UINT indexCount) -> void {
		NoConst(cmdQueue).DrawIndexed(indexCount, 0, 0);
	}

	auto SetRenderTargets(const ID3D11DeviceContext1& cmdQueue, std::span<ID3D11RenderTargetView* const*> colorTargets, ID3D11DepthStencilView* depthTarget) -> void {
		const u32 size = colorTargets.size();
		Assert(size <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		NoConst(cmdQueue).OMSetRenderTargets(size, colorTargets[0], depthTarget);
	}

	auto SetRenderTarget(const ID3D11DeviceContext1& cmdQueue, ID3D11RenderTargetView* const* colorTarget, ID3D11DepthStencilView* depthTarget) -> void {
		NoConst(cmdQueue).OMSetRenderTargets(1, colorTarget, depthTarget);
	}
}
