#pragma once

#include "DX11Layer.h"

// STL includes
#include <algorithm>
#include <iostream>
#include <string>

#include "Shaders/VertexShader.h"
#include "Shaders/PixelShader.h"

#include "Shaders/TexVertexShader.h"
#include "Shaders/TexPixelShader.h"

using namespace DirectX;
using namespace Renderer::DXLayer;
using namespace Microsoft::WRL;

struct VertexBuffer {
	ID3D11Buffer* buffer;
	D3D11_INPUT_ELEMENT_DESC inputElementDescription;
	UINT stride;
	UINT offset;
};

struct IndexBuffer {
	ID3D11Buffer* buffer;
	UINT offset;
};

struct ShaderData {
	const void* pShaderBytecodeWithInputSignature;
	SIZE_T BytecodeLength;
};

struct GPUMeshData {
	VertexBuffer vertexBuffer;
	u32 vertexCount;

	ID3D11Buffer* indexBuffer;
	u32 indexOffset;
	u32 indexCount;
};

struct PipelineState {
	ID3D11InputLayout* inputLayout = nullptr;

	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11Buffer* vertexConstantBuffers = nullptr;

	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11Buffer* pixelConstantBuffers = nullptr;

	ID3D11RasterizerState* rasterizerState = nullptr;
	ID3D11DepthStencilState* depthStencilState = nullptr;
	
	D3D11_PRIMITIVE_TOPOLOGY topology;
	short vertexConstantBuffersCount = 0;
	short pixelConstantBuffersCount = 0;
};

struct PerFrameBufferData {
	XMMATRIX viewMatrix;
	XMFLOAT3 cameraPosition;
	f32 spacing;
	XMFLOAT3 lightPosition;
};

// Shader resources
enum ConstanBuffer {
	CB_Appliation,
	CB_Frame,
	CB_Object,
	NumConstantBuffers
};

struct RendererState {
	HWND g_WindowHandle;

	// Direct3D device and swap chain.
	ComPtr<ID3D11Device1> device = nullptr;
	ComPtr<IDXGISwapChain1> swapChain = nullptr;
	CmdQueue cmdQueue = {};
	
	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView* defaultRenderTargetView = nullptr;
	// Depth/stencil view for use as a depth buffer.
	ID3D11DepthStencilView* defaultDepthStencilView = nullptr;
	// A texture to associate to the depth stencil view.
	ID3D11Texture2D* defaultDepthStencilBuffer = nullptr;

	D3D11_VIEWPORT g_Viewport = {0};

	ID3D11Resource* textureResource = nullptr;
	ID3D11ShaderResourceView* textureView = nullptr;
	ID3D11SamplerState* texSamplerState = nullptr;

	ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

	// Shader data
	ID3D11VertexShader* g_d3dSimpleVertexShader = nullptr;
	ID3D11PixelShader* g_d3dSimplePixelShader = nullptr;

	ID3D11VertexShader* g_d3dTexVertexShader = nullptr;
	ID3D11PixelShader* g_d3dTexPixelShader = nullptr;

	// Demo parameters
	XMMATRIX g_WorldMatrix;
	XMMATRIX g_ViewMatrix;
	XMMATRIX g_ProjectionMatrix;

	// ------- toto this probably should be separated
	ID3D11Buffer* vertexBuffers[20];
	ID3D11Buffer* indexBuffers[20];
	u32 vertexBuffersCount = 0;
	u32 indexBuffersCount = 0;

	ID3D11Buffer* zeroBuffer[4]; // TODO: check Sokol defines max to be 4
	ID3D11ShaderResourceView* zeroResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	ID3D11SamplerState* zeroSamplerStates[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	PipelineState pipelineStates[10];

	UINT backbufferWidth;
	UINT backbufferHeight;

	GPUMeshData GPUMeshData[2];
};

namespace Renderer {
	auto Initialize(HWND handle, u32 clientWidth, u32 clientHeight) -> RendererState;
	auto Clear(const CmdQueue& cmd, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil) -> void;
	auto CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto CreateVertexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr) -> ID3D11Buffer*;
	auto CreateConstantBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr) ->ID3D11Buffer*;
	auto CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest) ->ID3D11DepthStencilState*;
	auto CreateDefaultRasterizerState(ID3D11Device1* device) -> ID3D11RasterizerState*;
	auto CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels) ->ID3D11Texture2D*;
	auto CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height) -> ID3D11Texture2D*;
	auto CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture) -> ID3D11DepthStencilView*;
	auto DrawIndexed(const CmdQueue& cmd, int indexCount, int startIndex, int startVertex) -> void;
	auto CreateInputLayout(ID3D11Device1* device, D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc, UINT vertexLayoutDescLength, const BYTE* shaderBytecodeWithInputSignature, SIZE_T shaderBytecodeSize) -> ID3D11InputLayout*;
	auto CreateViewPort(f32 minX, f32 minY, f32 maxX, f32 maxY) -> D3D11_VIEWPORT;
	auto EnableDebug(const ID3D11Device1& device1) -> ID3D11Debug*;
}

template<class ShaderClass>
auto GetLatestProfile(RendererState* rs) -> std::string;

template<class ShaderClass>
auto CreateShader(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage) -> ShaderClass*;

template<class ShaderClass>
auto LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile) -> ShaderClass* {
	ID3DBlob* pShaderBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	ShaderClass* pShader = nullptr;

	std::string profile = _profile;
	if (profile == "latest") {
		profile = GetLatestProfile<ShaderClass>();
	}
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile.c_str(),
		flags, 0, &pShaderBlob, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			std::string errorMessage = (char*)pErrorBlob->GetBufferPointer();
			OutputDebugStringA(errorMessage.c_str());

			SafeRelease(pShaderBlob);
			SafeRelease(pErrorBlob);
		}

		return false;
	}

	pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr);

	SafeRelease(pShaderBlob);
	SafeRelease(pErrorBlob);

	return pShader;
}