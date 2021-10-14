#include "game.h"
#include "renderer.h"

/*
struct renderer_state {
	Device,
	DeviceContext,
	ViewMatrix,
	WorldMatrix,
	ConstantBuffers,
	VertexBuffer,
	IndexBuffer,
	VertexPosColor
};
*/

struct Vec3 {
	f32 x, y, z;
};

/*
auto CreateCubeMap() -> void {
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = description.width;
	texDesc.Height = description.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = texDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA pData[6];
	std::vector<vector4b> d[6]; // 6 images of type vector4b = 4 * unsigned char

	for (int cubeMapFaceIndex = 0; cubeMapFaceIndex < 6; cubeMapFaceIndex++)
	{
		d[cubeMapFaceIndex].resize(description.width * description.height);

		// fill with red color  
		std::fill(
			d[cubeMapFaceIndex].begin(),
			d[cubeMapFaceIndex].end(),
			vector4b(255, 0, 0, 255));

		pData[cubeMapFaceIndex].pSysMem = &d[cubeMapFaceIndex][0];// description.data;
		pData[cubeMapFaceIndex].SysMemPitch = description.width * 4;
		pData[cubeMapFaceIndex].SysMemSlicePitch = 0;
	}

	HRESULT hr = renderer->getDevice()->CreateTexture2D(&texDesc,
		description.data[0] ? &pData[0] : nullptr, &m_pCubeTexture);
	assert(hr == S_OK);

	hr = renderer->getDevice()->CreateShaderResourceView(
		m_pCubeTexture, &SMViewDesc, &m_pShaderResourceView);
	assert(hr == S_OK);
}
*/

const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
const UINT vertexStride = sizeof(VertexPosColor);
const UINT texVertexStride = sizeof(VertexPosUV);
const UINT offset = 0;

namespace Nickel {
	using namespace Renderer;

	auto SetPipelineState(const ID3D11DeviceContext1& cmd, RendererState* rs, PipelineState& pipeline) -> void {
		ID3D11DeviceContext1& cmdQueue = const_cast<ID3D11DeviceContext1&>(cmd);
		cmdQueue.RSSetState(pipeline.rasterizerState);
		cmdQueue.OMSetDepthStencilState(pipeline.depthStencilState, 1);

		// deviceCtx->OMSetBlendState(); // TODO

		cmdQueue.IASetPrimitiveTopology(pipeline.topology);
		cmdQueue.IASetInputLayout(pipeline.inputLayout);
		cmdQueue.VSSetShader(pipeline.vertexShader, nullptr, 0);

		if (pipeline.vertexConstantBuffersCount == 0)
			cmdQueue.VSSetConstantBuffers(0, ArrayCount(rs->zeroBuffer), rs->zeroBuffer);
		else
			cmdQueue.VSSetConstantBuffers(0, pipeline.vertexConstantBuffersCount, (ID3D11Buffer* const*)pipeline.vertexConstantBuffers);

		if (pipeline.pixelShader != nullptr)
			cmdQueue.PSSetShader(pipeline.pixelShader, nullptr, 0);
		// cmdQueue->PSSetConstantBuffers(); // TODO

		cmdQueue.RSSetViewports(1, &rs->g_Viewport);
	}

	inline auto SetVertexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* vertexBuffer, UINT stride, UINT offset) -> void {
		NoConst(cmdQueue).IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	inline auto SetIndexBuffer(const ID3D11DeviceContext1& cmdQueue, ID3D11Buffer* indexBuffer, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT, u32 offset = 0) -> void {
		NoConst(cmdQueue).IASetIndexBuffer(indexBuffer, format, offset);
	}

	inline auto DrawIndexed(const ID3D11DeviceContext1& cmdQueue, UINT indexCount) -> void {
		NoConst(cmdQueue).DrawIndexed(indexCount, 0, 0);
	}

	auto DrawBunny(const Nickel::Renderer::DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		SetPipelineState(cmdQueue, rs, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[0];

		cmdQueue.PSSetShaderResources(0, 1, &rs->textureView);
		cmdQueue.PSSetSamplers(0, 1, &rs->texSamplerState);

		SetIndexBuffer(cmdQueue, mesh->indexBuffer);
		SetVertexBuffer(cmdQueue, mesh->vertexBuffer.buffer, texVertexStride, offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto DrawLight(const DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		SetPipelineState(cmdQueue, rs, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[0];

		cmdQueue.PSSetShaderResources(0, 1, &rs->textureView);
		cmdQueue.PSSetSamplers(0, 1, &rs->texSamplerState);

		SetIndexBuffer(cmdQueue, mesh->indexBuffer);
		SetVertexBuffer(cmdQueue, mesh->vertexBuffer.buffer, texVertexStride, offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto DrawSuzanne(const DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		SetPipelineState(cmdQueue, rs, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[1];

		cmdQueue.PSSetShaderResources(0, ArrayCount(rs->zeroResourceViews), rs->zeroResourceViews);
		cmdQueue.PSSetSamplers(0, ArrayCount(rs->zeroSamplerStates), rs->zeroSamplerStates);

		SetIndexBuffer(cmdQueue, mesh->indexBuffer);
		SetVertexBuffer(cmdQueue, rs->GPUMeshData[1].vertexBuffer.buffer, vertexStride, offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto ApplyPipeline(ID3D11Device1* device, PipelineState* pipeline) -> void {
		assert(device != nullptr);
		assert(pipeline != nullptr);

		// TODO
		// pipeline->inputLayout = CreateInputLayout(device, layoutDescription.desc vertexPosUVLayoutDesc, layoutDescription.Length ArrayCount(vertexPosUVLayoutDesc), shader.bytecode g_TexVertexShader, shader.bytecodeLength ArrayCount(g_TexVertexShader));
	}

	auto GetVertexPosUVFromModelData(MeshData* data) -> std::vector<VertexPosUV> {
		assert(data != nullptr);

		std::vector<VertexPosUV> result;
		f32 x, y, z;
		for (u32 i = 0; i < data->v.size() / 3; ++i) {
			x = -data->v[i * 3];
			y = data->v[i * 3 + 1];
			z = -data->v[i * 3 + 2];

			VertexPosUV vertexData = {
				XMFLOAT3(x, y, z),
				XMFLOAT3(-data->n[i * 3], data->n[i * 3 + 1], -data->n[i * 3 + 2]),
				//XMFLOAT3(clamp(0.0f, 1.0f, x), clamp(0.0f, 1.0f, y), clamp(0.0f, 1.0f, z))
				// XMFLOAT3(0.53333, 0.84705, 0.69019),
				XMFLOAT2(data->uv[i * 2], data->uv[i * 2 + 1])
			};

			result.push_back(vertexData);
		}

		return result;
	}

	auto GetVertexPosColorFromModelData(MeshData* data) -> std::vector<VertexPosColor> {
		assert(data != nullptr);

		std::vector<VertexPosColor> result;
		f32 x, y, z;
		for (u32 i = 0; i < data->v.size() / 3; ++i) {
			x = -data->v[i * 3];
			y = data->v[i * 3 + 1];
			z = -data->v[i * 3 + 2];

			VertexPosColor vertexData = {
				XMFLOAT3(x, y, z),
				XMFLOAT3(-data->n[i * 3], data->n[i * 3 + 1], -data->n[i * 3 + 2]),
				XMFLOAT3(0.53333f, 0.84705f, 0.69019f)
			};

			result.push_back(vertexData);
		}

		return result;
	}

	auto Nickel::LoadObjMeshData(MeshData& meshData, const std::string& path) -> void {
		auto loader = ObjLoader();
		ObjFileMemory objFile = loader.DEBUG_ReadEntireFile(path.c_str());
		loader.LoadObjMesh(objFile, meshData);
	}

	auto LoadBunnyMesh(MeshData& meshData) -> void {
		LoadObjMeshData(meshData, "Data/Models/bny.obj");
	}

	auto LoadSuzanneModel(MeshData& meshData) -> void {
		LoadObjMeshData(meshData, "Data/Models/Suzanne.obj");

		/*
		auto device = rs->device.Get();

		auto meshData = MeshData();
		LoadObjMeshData(meshData, "Data/Models/Suzanne.obj");
		auto vertexFormatData = GetVertexPosColorFromModelData(&meshData);

		GPUMeshData mesh = {

		};
		mesh.indexCount = meshData.i.size();


		mesh.vertexCount = vertexFormatData.size();

		mesh.vertexBuffer.buffer = Renderer::CreateVertexBuffer(device, sizeof(vertexFormatData[0]) * mesh.vertexCount);

		D3D11_SUBRESOURCE_DATA resourceData3 = {0};
		resourceData3.pSysMem = vertexFormatData.data();
		rs->deviceCtx->UpdateSubresource1(mesh.vertexBuffer.buffer, 0, nullptr, resourceData3.pSysMem, 0, 0, 0);

		D3D11_SUBRESOURCE_DATA resourceData4 = {0};
		resourceData4.pSysMem = meshData.i.data();

		u32 byteWidthSize = sizeof(meshData.i[0]) * mesh.indexCount;
		// mesh.indexBuffer = Renderer::CreateIndexBuffer(device, byteWidthSize, &resourceData4);

		return mesh;
		*/
	}

	void LoadMeshAndSetup(const std::string& path) {
		MeshData meshData;
		LoadObjMeshData(meshData, path);
	}

	//

	auto Initialize(GameMemory* memory, RendererState* rs) -> void {
		assert(memory != nullptr);
		assert(rs != nullptr);
		assert(rs->device);
		assert(rs->cmdQueue.queue);

		ID3D11Device1* device = rs->device.Get();

		if (!LoadContent(rs)) {
			// TODO: log error
		}

		rs->defaultDepthStencilBuffer = DXLayer::CreateDepthStencilTexture(device, rs->backbufferWidth, rs->backbufferHeight);
		assert(rs->defaultDepthStencilBuffer != nullptr);
		rs->defaultDepthStencilView = DXLayer::CreateDepthStencilView(device, rs->defaultDepthStencilBuffer);

		auto depthStencilState = DXLayer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
		auto rasterizerState = DXLayer::CreateDefaultRasterizerState(device);

		auto pip = &rs->pipelineStates[0];
		pip->depthStencilState = depthStencilState;
		pip->rasterizerState = rasterizerState;
		pip->inputLayout = DXLayer::CreateInputLayout(device, vertexPosUVLayoutDesc, ArrayCount(vertexPosUVLayoutDesc), g_TexVertexShader, ArrayCount(g_TexVertexShader));
		pip->vertexShader = rs->g_d3dTexVertexShader;
		pip->vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
		pip->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
		pip->pixelShader = rs->g_d3dTexPixelShader;
		pip->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// ApplyPipeline(device, &pip); // TODO

		auto pip2 = &rs->pipelineStates[1];
		pip2->depthStencilState = depthStencilState;
		pip2->rasterizerState = rasterizerState;
		pip2->inputLayout = DXLayer::CreateInputLayout(device, vertexPosColorLayoutDesc, ArrayCount(vertexPosColorLayoutDesc), g_SimpleVertexShader, ArrayCount(g_SimpleVertexShader));
		pip2->vertexShader = rs->g_d3dSimpleVertexShader;
		pip2->vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
		pip2->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
		pip2->pixelShader = rs->g_d3dSimplePixelShader;
		pip2->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		D3D11_RENDER_TARGET_BLEND_DESC1 blendDescription = { 0 };
		D3D11_BLEND_DESC1 stateDesc = { 0 };
		stateDesc.AlphaToCoverageEnable = true;
		stateDesc.IndependentBlendEnable = true;
		for (int i = 0; i < ArrayCount(stateDesc.RenderTarget); i++) {
			stateDesc.RenderTarget[i] = blendDescription;
		}

		// TODO: this is only dummy test code
		ID3D11BlendState1* state;
		device->CreateBlendState1(&stateDesc, &state);
		const FLOAT blendFactor[4] = { 0 };
		// rs->deviceCtx->OMSetBlendState(state, blendFactor, 0);

		ID3D11DeviceContext1* cmdQueue = rs->cmdQueue.queue.Get();
		cmdQueue->OMSetRenderTargets(1, &rs->defaultRenderTargetView, nullptr);
		cmdQueue->OMSetRenderTargets(1, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
	}

	auto UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void {
		// GameState* gs = (GameState*)memory;

		Vec3 cameraPos;
		cameraPos.x = 0.0f;
		cameraPos.y = 0.0f;
		cameraPos.z = -10.0f;

		XMVECTOR eyePosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1);
		XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);

		static float angle = -90.0f;
		float radius = 2.1f;
		angle += 1.2f;
		XMFLOAT3 lightPos = XMFLOAT3(radius * cos(XMConvertToRadians(angle)), 0.0f, radius * sin(XMConvertToRadians(angle)));

		PerFrameBufferData frameData;
		frameData.viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
		frameData.cameraPosition = XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
		frameData.lightPosition = lightPos;

		auto& cmdQueue = rs->cmdQueue;

		cmdQueue.queue->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Frame], 0, nullptr, &frameData, 0, 0, 0);

		// RENDER ---------------------------
		const FLOAT clearColor[4] = { 0.13333f, 0.13333f, 0.13333f, 1.0f };
		DXLayer::Clear(cmdQueue, rs->defaultRenderTargetView, rs->defaultDepthStencilView, clearColor, 1.0f, 0);

		assert(rs->defaultDepthStencilView != nullptr);
		SetDefaultPass(cmdQueue, &rs->defaultRenderTargetView, *rs->defaultDepthStencilView);

		rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(12.0f, 12.0f, 12.0f));
		rs->g_WorldMatrix *= XMMatrixTranslation(0.0f, -2.0f, 0.0f);
		DrawBunny(cmdQueue, rs, rs->pipelineStates[0]);

		rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(2.0f, 2.0f, 2.0f));
		rs->g_WorldMatrix *= XMMatrixTranslation(lightPos.x, lightPos.y, lightPos.z);
		DrawLight(cmdQueue, rs, rs->pipelineStates[0]);

		rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.5f, 0.5f, 0.5f));
		rs->g_WorldMatrix *= XMMatrixTranslation(radius * cos(XMConvertToRadians(180.0f)), 0.0f, radius * sin(XMConvertToRadians(180.0f)));
		DrawSuzanne(cmdQueue, rs, rs->pipelineStates[1]);

		//deviceCtx->Draw(g_vertexCount, 0);

		rs->swapChain->Present(1, 0);
	}

	auto LoadContent(RendererState* rs) -> bool {
		assert(rs != nullptr);
		assert(rs->device != nullptr);
		assert(rs->cmdQueue.queue != nullptr);

		auto device = rs->device.Get();

		{ // Bunny
			LoadMeshAndSetup("Data/Models/bny.obj");

			MeshData meshData;
			LoadBunnyMesh(meshData);

			auto vertexFormatData = GetVertexPosUVFromModelData(&meshData);
			auto vertexCount = static_cast<u32>(vertexFormatData.size());
			auto indexCount = static_cast<u32>(meshData.i.size());

			auto vertexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexFormatData.data() };
			auto indexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = meshData.i.data() };
			auto indexByteWidthSize = sizeof(meshData.i[0]) * indexCount;

			// TODO: initialize vertex buffer fully!!
			auto vertexBuffer = VertexBuffer{
				.buffer = Renderer::DXLayer::CreateVertexBuffer(device, (sizeof(vertexFormatData[0]) * vertexCount), &vertexSubresource),
				.inputElementDescription{},
				.stride{},
				.offset{}
			};

			auto gpuMeshData = GPUMeshData{
				.vertexBuffer = vertexBuffer,
				.vertexCount = vertexCount,

				.indexBuffer = Renderer::DXLayer::CreateIndexBuffer(device, indexByteWidthSize, &indexSubresource),
				.indexCount = indexCount
			};

			rs->GPUMeshData[0] = gpuMeshData;
		}

		{ // Suzanne
			MeshData meshData;
			LoadSuzanneModel(meshData);

			auto vertexFormatData = GetVertexPosColorFromModelData(&meshData); // TODO: change this to GetVertexPosUVFromModelData and make easy to debug
			auto vertexCount = static_cast<u32>(vertexFormatData.size());
			auto indexCount = static_cast<u32>(meshData.i.size());

			auto vertexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexFormatData.data() };
			auto indexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = meshData.i.data() };
			auto indexByteWidthSize = sizeof(meshData.i[0]) * indexCount;

			auto vertexBuffer = VertexBuffer{
				.buffer = DXLayer::CreateVertexBuffer(device, (sizeof(vertexFormatData[0]) * vertexCount), &vertexSubresource)
			};

			auto gpuMeshData = GPUMeshData{
				.vertexBuffer = vertexBuffer,
				.vertexCount = vertexCount,

				.indexBuffer = DXLayer::CreateIndexBuffer(device, indexByteWidthSize, &indexSubresource),
				.indexCount = indexCount
			};

			rs->GPUMeshData[1] = gpuMeshData;
		}

		// Create the constant buffers for the variables defined in the vertex shader.
		rs->g_d3dConstantBuffers[CB_Appliation] = DXLayer::CreateConstantBuffer(device, sizeof(XMMATRIX));
		rs->g_d3dConstantBuffers[CB_Object] = DXLayer::CreateConstantBuffer(device, sizeof(XMMATRIX));
		rs->g_d3dConstantBuffers[CB_Frame] = DXLayer::CreateConstantBuffer(device, sizeof(PerFrameBufferData));

		// vertex shader
		HRESULT hr = rs->device->CreateVertexShader(g_SimpleVertexShader, sizeof(g_SimpleVertexShader), nullptr, &rs->g_d3dSimpleVertexShader);
		if (FAILED(hr)) {
			return false;
		}

		// vertex shader2
		hr = rs->device->CreateVertexShader(g_TexVertexShader, sizeof(g_TexVertexShader), nullptr, &rs->g_d3dTexVertexShader);
		if (FAILED(hr)) {
			return false;
		}

		// Load the compiled pixel shader.
		hr = rs->device->CreatePixelShader(g_SimplePixelShader, sizeof(g_SimplePixelShader), nullptr, &rs->g_d3dSimplePixelShader);
		if (FAILED(hr)) {
			return false;
		}

		hr = rs->device->CreatePixelShader(g_TexPixelShader, sizeof(g_TexPixelShader), nullptr, &rs->g_d3dTexPixelShader);
		if (FAILED(hr)) {
			return false;
		}

		// create Texture
		hr = CreateWICTextureFromFile(*rs->device.GetAddressOf(),
			L"Data/Textures/matcap.jpg", //tex.png
			&rs->textureResource, &rs->textureView);
		if (FAILED(hr)) {
			return false;
		}

		// Setup the projection matrix.
		RECT clientRect;
		GetClientRect(rs->g_WindowHandle, &clientRect);

		// Compute the exact client dimensions.
		// This is required for a correct projection matrix.
		float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
		float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

		rs->g_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

		rs->cmdQueue.queue->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Appliation], 0, nullptr, &rs->g_ProjectionMatrix, 0, 0, 0);

		return true;
	}

	auto SetDefaultPass(const DXLayer::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void {
		assert(cmd.queue != nullptr);
		cmd.queue->OMSetRenderTargets(1, renderTargetView, &depthStencilView);
	}
}

// DrawMesh(mesh, material)
struct RenderPassDescription {

};