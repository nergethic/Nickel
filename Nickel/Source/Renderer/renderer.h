#pragma once

#include "../Window.h"
#include "Direct3D11/DX11Layer.h"
#include "../ShaderProgram.h"

// STL includes
#include <algorithm>
#include <iostream>
#include <string>

#include "../Shaders/ColorPixelShader.h"
#include "../Shaders/LineVertexShader.h"

#include "../Shaders/VertexShader.h"
#include "../Shaders/PixelShader.h"

#include "../Shaders/TexVertexShader.h"
#include "../Shaders/TexPixelShader.h"

#include "../Shaders/BackgroundVertexShader.h"
#include "../Shaders/BackgroundPixelShader.h"

#include "../Shaders/PbrVertexShader.h"
#include "../Shaders/PbrPixelShader.h"

#include "../Shaders/ConvoluteBackgroundPixelShader.h"

//#include "Shaders/BackgroundVertexShader.h"

#include "../Camera.h" // TODO: move to scene manager
#include "../Math.h"
#include "../Mesh.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"

#include "RendererInterface.h"

using namespace DirectX;
using namespace Nickel::Renderer;
using namespace Microsoft::WRL;

struct ShaderData {
	const void* pShaderBytecodeWithInputSignature;
	SIZE_T BytecodeLength;
};

struct GPUMeshData {
	VertexBuffer vertexBuffer;
	u64 vertexCount;

	IndexBuffer indexBuffer;
	u64 indexCount;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct PipelineState { // rasterizer, blend, depth, stencil
	ID3D11RasterizerState* rasterizerState = nullptr;
	ID3D11DepthStencilState* depthStencilState = nullptr;
	// D3D11_RENDER_TARGET_BLEND_DESC1 ?? 
};

struct PerApplicationData {
	XMMATRIX projectionMatrix;
	XMFLOAT3 clientData;
};

//__declspec(align(16))
// alignas(16)
struct PerFrameBufferData {
	XMMATRIX viewMatrix;
	XMFLOAT3 cameraPosition;
	f32 spacing;
	XMFLOAT3 lightPosition;
};

struct PerObjectBufferData {
	XMMATRIX modelMatrix;
	XMMATRIX viewProjectionMatrix;
	XMMATRIX modelViewProjectionMatrix;
};

struct alignas(16) LineBufferData {
	f32 thickness;
	i32 miter;
	f64 spacing;
};

//alignas(16)
// #pragma pack(4)
// alignas(16)
struct PbrPixelBufferData {
	XMFLOAT4 lightPositions[4]; // 48 3*[16]
	XMFLOAT4 lightColors[4]; // 48 3*[16]
	XMFLOAT4 albedoFactor; // 4
	f32 metallic; // 8
	f32 roughness; // 12
	f32 ao; // 16
	f32 padding;
};

enum class ConstantBufferType {
	CB_Appliation,
	CB_Frame,
	CB_Object,
	NumConstantBuffers
};

struct Transform {
	Nickel::Vec3 position;
	Nickel::Vec3 scale;
	Nickel::Vec3 rotation;
};

struct TextureData {
	std::string name;
};

struct ConstantBuffer {
	ComPtr<ID3D11Buffer> buffer = nullptr;
	u32 index = 0;

	template <typename T>
	auto Update(ID3D11DeviceContext1* cmd, const T& data) -> void {
		cmd->UpdateSubresource1(buffer.Get(), 0, nullptr, std::addressof(data), 0, 0, 0);
	}
};

struct Material {
	Nickel::Renderer::DX11Layer::ShaderProgram* program;
	PipelineState pipelineState;
	D3D11_CULL_MODE overrideCullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	std::vector<DX11Layer::TextureDX11> textures;
	
	ConstantBuffer vertexConstantBuffer;
	ConstantBuffer pixelConstantBuffer;
};

struct DescribedMesh {
	Transform transform;
	Nickel::MeshData mesh;
	GPUMeshData gpuData;
	Material material;
};

struct GfxDevice {

};

struct RendererState {
	HWND g_WindowHandle;

	// Direct3D device and swap chain.
	ComPtr<ID3D11Device1> device = nullptr;
	ComPtr<IDXGISwapChain1> swapChain = nullptr;
	DX11Layer::CmdQueue cmdQueue = {};
	
	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView* defaultRenderTargetView = nullptr;
	// Depth/stencil view for use as a depth buffer.
	ID3D11DepthStencilView* defaultDepthStencilView = nullptr;
	// A texture to associate to the depth stencil view.
	ID3D11Texture2D* defaultDepthStencilBuffer = nullptr;

	D3D11_VIEWPORT g_Viewport = {0};

	DX11Layer::TextureDX11 albedoTexture;
	DX11Layer::TextureDX11 normalTexture;
	DX11Layer::TextureDX11 aoTexture;
	DX11Layer::TextureDX11 metalRoughnessTexture;
	DX11Layer::TextureDX11 emissiveTexture;
	DX11Layer::TextureDX11 matCapTexture;
	DX11Layer::TextureDX11 debugBoxTexture;
	DX11Layer::TextureDX11 radianceTexture;
	DX11Layer::TextureDX11 brdfLUT;

	ID3D11Buffer* g_d3dConstantBuffers[(u32)ConstantBufferType::NumConstantBuffers];

	// Demo parameters
	XMMATRIX g_WorldMatrix;
	XMMATRIX g_ViewMatrix;
	XMMATRIX g_ProjectionMatrix;

	// ------- todo this probably should be separated
	ID3D11Buffer* vertexBuffers[20];
	ID3D11Buffer* indexBuffers[20];
	u32 vertexBuffersCount = 0;
	u32 indexBuffersCount = 0;

	ID3D11Buffer* zeroBuffer[4]; // TODO: check Sokol defines max to be 4
	ID3D11ShaderResourceView* zeroResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	ID3D11SamplerState* zeroSamplerStates[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	// PipelineState pipelineStates[10];

	UINT backbufferWidth;
	UINT backbufferHeight;

	DX11Layer::ShaderProgram pbrProgram;
	DX11Layer::ShaderProgram lineProgram;
	DX11Layer::ShaderProgram simpleProgram;
	DX11Layer::ShaderProgram textureProgram;
	DX11Layer::ShaderProgram convoluteIrradianceBackgroundProgram;

	Material pbrMat;
	Material lineMat;
	Material simpleMat;
	Material textureMat;
	Material convoluteIrradianceBackgroundMat;

	DescribedMesh debugCube;
	std::vector<DescribedMesh> bunny;
	DescribedMesh suzanne;
	DescribedMesh light;
	DescribedMesh debugBoxTextured;
	std::vector<DescribedMesh> lines = std::vector<DescribedMesh>(30);

	DescribedMesh* sceneMeshes[3];

	std::unique_ptr<Nickel::Camera> mainCamera;
};

namespace Nickel::Renderer {
	auto Initialize(HWND handle, u32 clientWidth, u32 clientHeight)->RendererState;
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