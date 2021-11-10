#pragma once

#include "DX11Layer.h"
#include "ShaderProgram.h"

// STL includes
#include <algorithm>
#include <iostream>
#include <string>

#include "Shaders/ColorPixelShader.h"
#include "Shaders/LineVertexShader.h"

#include "Shaders/VertexShader.h"
#include "Shaders/PixelShader.h"

#include "Shaders/TexVertexShader.h"
#include "Shaders/TexPixelShader.h"

#include "Shaders/BackgroundVertexShader.h"
#include "Shaders/BackgroundPixelShader.h"

//#include "Shaders/BackgroundVertexShader.h"

#include "Camera.h" // TODO: move to scene manager
#include "Math.h"

using namespace DirectX;
using namespace Nickel::Renderer;
using namespace Microsoft::WRL;

struct VertexBuffer {
	std::unique_ptr<ID3D11Buffer, DxDeleter> buffer;
	UINT stride;
	UINT offset;
	bool isDynamic;

	template <typename VertexT>
	void Create(ID3D11Device1* device, std::span<VertexT> vertexData, bool dynamic) {
		Assert(buffer.get() == nullptr);
		auto data = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexData.data() };

		auto rawBuffer = DXLayer::CreateVertexBuffer(device, (sizeof(VertexT) * vertexData.size()), dynamic, &data);
		buffer = std::unique_ptr<ID3D11Buffer, DxDeleter>(rawBuffer);
		stride = sizeof(VertexT);
		offset = 0;
		isDynamic = dynamic;
	}

	template <typename VertexT>
	void Update(ID3D11DeviceContext1* ctx, std::span<VertexT const*> vertexData) {
		if constexpr (_DEBUG) {
			Assert(isDynamic);
			D3D11_BUFFER_DESC buffer_desc;
			buffer->GetDesc(&buffer_desc);
			Assert(buffer_desc.Usage == D3D11_USAGE_DYNAMIC && buffer_desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE);
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		void* dataPtr;
		ctx->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr = mappedResource.pData;
		memcpy(dataPtr, vertexData.data(), vertexData.size());
		ctx->Unmap(buffer.get(), 0);
	}
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
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct PipelineState { // rasterizer, blend, depth, stencil
	//ID3D11InputLayout* inputLayout = nullptr;

	//ID3D11VertexShader* vertexShader = nullptr;
	//ID3D11Buffer* vertexConstantBuffers = nullptr;
	//ID3D11PixelShader* pixelShader = nullptr;
	//ID3D11Buffer* pixelConstantBuffers = nullptr;

	ID3D11RasterizerState* rasterizerState = nullptr;
	ID3D11DepthStencilState* depthStencilState = nullptr;
	// D3D11_RENDER_TARGET_BLEND_DESC1 ?? 
	
	//D3D11_PRIMITIVE_TOPOLOGY topology;
	//short vertexConstantBuffersCount = 0;
	//short pixelConstantBuffersCount = 0;
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

// Shader resources
enum class ConstantBuffer {
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

struct TextureDX11 {
	ID3D11Resource* resource;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* samplerState;
};

struct Material {
	Nickel::Renderer::DXLayer::ShaderProgram* program;
	PipelineState pipelineState;
	D3D11_CULL_MODE overrideCullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	std::vector<TextureDX11> textures;
	ComPtr<ID3D11Buffer> constantBuffer;
};

#include "Mesh.h"
struct DescribedMesh {
	Transform transform;
	Nickel::MeshData mesh;
	GPUMeshData* gpuData; // TODO: figure out better structure design, if someone assigns different gpuData then mesh won't correspond to it
	Material material;
};

struct RendererState {
	HWND g_WindowHandle;

	// Direct3D device and swap chain.
	ComPtr<ID3D11Device1> device = nullptr;
	ComPtr<IDXGISwapChain1> swapChain = nullptr;
	DXLayer::CmdQueue cmdQueue = {};
	
	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView* defaultRenderTargetView = nullptr;
	// Depth/stencil view for use as a depth buffer.
	ID3D11DepthStencilView* defaultDepthStencilView = nullptr;
	// A texture to associate to the depth stencil view.
	ID3D11Texture2D* defaultDepthStencilBuffer = nullptr;

	D3D11_VIEWPORT g_Viewport = {0};

	TextureDX11 matCapTexture;
	TextureDX11 skyboxTexture;

	ID3D11Buffer* g_d3dConstantBuffers[(u32)ConstantBuffer::NumConstantBuffers];

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

	GPUMeshData gpuMeshData[2];
	GPUMeshData skyboxMeshData;
	GPUMeshData debugCubeGpuMeshData;
	std::vector<GPUMeshData> linesGPUData = std::vector<GPUMeshData>(30);

	DXLayer::ShaderProgram lineProgram;
	DXLayer::ShaderProgram simpleProgram;
	DXLayer::ShaderProgram textureProgram;
	DXLayer::ShaderProgram backgroundProgram;

	Material lineMat;
	Material simpleMat;
	Material textureMat;
	Material backgroundMat;

	DescribedMesh debugCube;
	DescribedMesh bunny;
	DescribedMesh suzanne;
	DescribedMesh light;
	DescribedMesh skybox;
	std::vector<DescribedMesh> lines = std::vector<DescribedMesh>(30);

	DescribedMesh* sceneMeshes[3];

	std::unique_ptr<Nickel::Camera> mainCamera;
};

namespace Nickel {
	namespace Renderer {
		auto Initialize(HWND handle, u32 clientWidth, u32 clientHeight) -> RendererState;
	}
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