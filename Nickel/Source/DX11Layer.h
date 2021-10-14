#pragma once

#pragma warning(push, 0) // ignores warnings from external headers
// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib") // TODO: check why this doesn't have stuff from dxgi1_3.h (CreateDXGIFactory2)
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

// DirectX includes
#include "platform.h"
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <WICTextureLoader.h>
#include <dxgi.h>
#include <dxgi1_3.h>

#include <wrl/client.h>
#pragma warning(pop)

#include "platform.h"

using namespace Microsoft::WRL;

namespace Nickel::Renderer::DXLayer {
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

	auto CreateVertexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr)->ID3D11Buffer*;
	auto CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr)->ID3D11Buffer*;
	auto CreateConstantBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr)->ID3D11Buffer*;
	auto CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData = nullptr)->ID3D11Buffer*;
	auto Clear(const CmdQueue& cmd, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void;
	auto CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest)->ID3D11DepthStencilState*;
	auto CreateDefaultRasterizerState(ID3D11Device1* device)->ID3D11RasterizerState*;
	auto CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels)->ID3D11Texture2D*;
	auto CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height)->ID3D11Texture2D*;
	auto CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture)->ID3D11DepthStencilView*;
	auto DrawIndexed(const CmdQueue& cmd, int indexCount, int startIndex, int startVertex) -> void;
	auto CreateInputLayout(ID3D11Device1* device, D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc, UINT vertexLayoutDescLength, const BYTE* shaderBytecodeWithInputSignature, SIZE_T shaderBytecodeSize)->ID3D11InputLayout*;
	auto CreateViewPort(f32 minX, f32 minY, f32 maxX, f32 maxY)->D3D11_VIEWPORT;
	auto EnableDebug(const ID3D11Device1& device1)->ID3D11Debug*;
}