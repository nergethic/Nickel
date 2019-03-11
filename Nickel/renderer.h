#pragma once
// DirectX includes
#include "platform.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// STL includes
#include <algorithm>
#include <iostream>
#include <string>

// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

#include "VertexShader.h"
#include "PixelShader.h"

using namespace DirectX;

//static 

// Vertex data for a colored cube.
struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
	XMFLOAT3 Normal;
};

struct PerFrameBufferData {
	XMMATRIX viewMatrix;
	XMFLOAT3 cameraPosition;
	f32 spacing;
	XMFLOAT3 lightPosition;
};

// Shader resources
enum ConstanBuffer
{
	CB_Appliation,
	CB_Frame,
	CB_Object,
	NumConstantBuffers
};

struct RendererState {
	HWND g_WindowHandle;

	// Direct3D device and swap chain.
	ID3D11Device* g_d3dDevice = nullptr;
	ID3D11DeviceContext* g_d3dDeviceContext = nullptr;
	IDXGISwapChain* g_d3dSwapChain = nullptr;
#ifdef _DEBUG
	ID3D11Debug *d3dDebug = nullptr;
#endif // _DEBUG

	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr;
	// Depth/stencil view for use as a depth buffer.
	ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr;
	// A texture to associate to the depth stencil view.
	ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;

	// Define the functionality of the depth/stencil stages.
	ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr;
	// Define the functionality of the rasterizer stage.
	ID3D11RasterizerState* g_d3dRasterizerState = nullptr;
	D3D11_VIEWPORT g_Viewport = {0};

	// Vertex buffer data
	ID3D11InputLayout* g_d3dInputLayout = nullptr;

	ID3D11Buffer* g_d3dVertexBuffer1 = nullptr;
	ID3D11Buffer* g_d3dIndexBuffer1 = nullptr;
	ID3D11Buffer* g_d3dVertexBuffer2 = nullptr;
	ID3D11Buffer* g_d3dIndexBuffer2 = nullptr;

	ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

	// Shader data
	ID3D11VertexShader* g_d3dVertexShader = nullptr;
	ID3D11PixelShader* g_d3dPixelShader = nullptr;

	// Demo parameters
	XMMATRIX g_WorldMatrix;
	XMMATRIX g_ViewMatrix;
	XMMATRIX g_ProjectionMatrix;

	u32 g_indexCount1  = 0;
	u32 g_vertexCount1 = 0;
	u32 g_indexCount2  = 0;
	u32 g_vertexCount2 = 0;
};

// Safely release a COM object.
template<typename T>
inline void SafeRelease(T& ptr)
{
	if (ptr != NULL)
	{
		ptr->Release();
		ptr = NULL;
	}
};

DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync);
void Clear(RendererState* rs, const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);