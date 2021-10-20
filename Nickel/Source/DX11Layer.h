#pragma once

#include "DirectXIncludes.h"
#include "platform.h"

using namespace Microsoft::WRL;

namespace Nickel::Renderer::DXLayer {
	enum class ClearFlag { // TODO: move to base renderer layer
		CLEAR_COLOR   = 1 << 0,
		CLEAR_DEPTH   = 1 << 1,
		CLEAR_STENCIL = 1 << 2
	};

	inline ClearFlag operator|(ClearFlag a, ClearFlag b) {
		return static_cast<ClearFlag>(static_cast<u32>(a) | static_cast<u32>(b));
	}

	struct CmdQueue {
		ComPtr<ID3D11DeviceContext1> queue;
		ComPtr<ID3D11Debug> debug;
	};

	const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	auto QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync) -> DXGI_RATIONAL;
	auto GetHighestQualitySampleLevel(ID3D11Device1* device, DXGI_FORMAT format) -> UINT;
	auto CreateDevice() -> std::pair<ID3D11Device*, ID3D11DeviceContext*>;
	auto CreateSwapChain(HWND windowHandle, ID3D11Device1* device, int clientWidth, int clientHeight) -> IDXGISwapChain1*;

	auto CreateVertexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto CreateConstantBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr)->ID3D11Buffer*;
	auto CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto Clear(const CmdQueue& cmd, u32 clearFlag, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void;
	auto CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest) -> ID3D11DepthStencilState*;
	auto CreateDefaultRasterizerState(ID3D11Device1* device)->ID3D11RasterizerState*;
	auto CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels) -> ID3D11Texture2D*;
	auto CreateSamplerState(ID3D11Device* device, const D3D11_SAMPLER_DESC& desc) -> ID3D11SamplerState*;
	auto CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height) -> ID3D11Texture2D*;
	auto CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture) -> ID3D11DepthStencilView*;
	auto Draw(const CmdQueue& cmd, int indexCount, int startVertex) -> void;
	auto DrawIndexed(const CmdQueue& cmd, int indexCount, int startIndex, int startVertex) -> void;
	auto CreateInputLayout(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> shaderBytecodeWithInputSignature)->ID3D11InputLayout*;
	auto CreateViewPort(f32 minX, f32 minY, f32 maxX, f32 maxY)->D3D11_VIEWPORT;
	auto EnableDebug(const ID3D11Device1& device1, bool shouldBeVerbose)->ID3D11Debug*;

	auto SetRenderTargets(const ID3D11DeviceContext1& cmdQueue, std::span<ID3D11RenderTargetView* const*> colorTargets, ID3D11DepthStencilView* depthTarget = nullptr) -> void;
	auto SetRenderTarget(const ID3D11DeviceContext1& cmdQueue, ID3D11RenderTargetView* const* colorTarget, ID3D11DepthStencilView* depthTarget = nullptr) -> void;

	auto SetVertexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* vertexBuffer, UINT stride, UINT offset) -> void;
	auto SetIndexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* indexBuffer, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT, u32 offset = 0) -> void;
	auto DrawIndexed(const ID3D11DeviceContext1& cmdQueue, UINT indexCount) -> void;
}