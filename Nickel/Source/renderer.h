#pragma once

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

// STL includes
#include <algorithm>
#include <iostream>
#include <string>

#include "Shaders/VertexShader.h"
#include "Shaders/PixelShader.h"

#include "Shaders/TexVertexShader.h"
#include "Shaders/TexPixelShader.h"

using namespace DirectX;

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

struct Mesh {
	VertexBuffer vertexBuffer;
	u32 vertexCount;

	ID3D11Buffer* indexBuffer;
	u32 indexOffset;
	u32 indexCount;
};

struct PipelineState {
	ID3D11RasterizerState* rasterizerState;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11Buffer* vertexConstantBuffers;
	short vertexConstantBuffersCount;
	ID3D11PixelShader* pixelShader;
	ID3D11Buffer* pixelConstantBuffers;
	short pixelConstantBuffersCount;
	D3D11_PRIMITIVE_TOPOLOGY topology;
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
	Microsoft::WRL::ComPtr<ID3D11Device1> device = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> deviceCtx = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain = nullptr;

#ifdef _DEBUG
	ID3D11Debug *d3dDebug = nullptr;
#endif // _DEBUG

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

	PipelineState pip;
	PipelineState pip2;

	UINT backbufferWidth;
	UINT backbufferHeight;

	Mesh meshes[2];
};

// Safely release a COM object.
template<typename T>
inline void SafeRelease(T& ptr) {
	if (ptr != nullptr) {
		ptr->Release();
		ptr = nullptr;
	}
};

DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync);
void Clear(RendererState* rs, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);
ID3D11Buffer* CreateBuffer(ID3D11Device1* device, D3D11_USAGE usage, UINT bindFlags, UINT byteWidthSize, UINT cpuAccessFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA* initialData = nullptr);
ID3D11Buffer* CreateVertexBuffer(RendererState* rs, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr);
ID3D11Buffer* CreateIndexBuffer(ID3D11Device1* device, u32 size, D3D11_SUBRESOURCE_DATA* initialData = nullptr);
ID3D11DepthStencilState* CreateDepthStencilState(ID3D11Device1* device, bool enableDepthTest, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool enableStencilTest);
ID3D11RasterizerState* CreateDefaultRasterizerState(ID3D11Device1* device);
ID3D11Texture2D* CreateTexture(ID3D11Device1* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlags, UINT mipLevels);
ID3D11Texture2D* CreateDepthStencilTexture(ID3D11Device1* device, UINT width, UINT height);
ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Device1* device, ID3D11Resource* depthStencilTexture);
UINT GetHighestQualitySampleLevel(ID3D11Device1* device, DXGI_FORMAT format);