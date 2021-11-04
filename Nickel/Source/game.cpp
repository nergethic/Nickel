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

struct Color {
	u8 r, g, b, a;
};

auto CreateCubeMap(RendererState& rs, u32 width, u32 height) -> void {
	auto texDesc = D3D11_TEXTURE2D_DESC{
		.Width = width,
		.Height = height,
		.MipLevels = 1,
		.ArraySize = 6,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = DXGI_SAMPLE_DESC{	
			.Count = 1,
			.Quality = 0,
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE
	};
	
	D3D11_SUBRESOURCE_DATA pData[6];
	std::vector<Color> imgs[6];

	for (int cubeMapFaceIdx = 0; cubeMapFaceIdx < 6; cubeMapFaceIdx++) {
		imgs[cubeMapFaceIdx].resize(width*height);

		// fill with red color  
		std::fill(
			imgs[cubeMapFaceIdx].begin(),
			imgs[cubeMapFaceIdx].end(),
			Color(255, 0, 0, 255));

		pData[cubeMapFaceIdx].pSysMem = &imgs[cubeMapFaceIdx][0]; // description.data;
		pData[cubeMapFaceIdx].SysMemPitch = width * 4;
		pData[cubeMapFaceIdx].SysMemSlicePitch = 0;
	}

	auto device = rs.device.Get();
	ID3D11Texture2D* cubeTexture;
	ASSERT_ERROR_RESULT(device->CreateTexture2D(&texDesc, &pData[0], &cubeTexture));

	auto resourceViewDesc = D3D11_SHADER_RESOURCE_VIEW_DESC{
		.Format = texDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE,
		.TextureCube = D3D11_TEXCUBE_SRV{
			.MostDetailedMip = 0,
			.MipLevels = texDesc.MipLevels
		}
	};

	ID3D11ShaderResourceView* srv;
	ASSERT_ERROR_RESULT(device->CreateShaderResourceView(cubeTexture, &resourceViewDesc, &srv));

	rs.cubeMap = TextureDX11{
			.resource = cubeTexture,
			.srv = srv,
			.samplerState = ResourceManager::GetDefaultSamplerState()
	};
}

const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
const UINT offset = 0;

namespace Nickel {
	using namespace Renderer;

	auto SetPipelineState(const ID3D11DeviceContext1& cmd, D3D11_VIEWPORT* viewport, const PipelineState& pipeline) -> void {
		ID3D11DeviceContext1& cmdQueue = const_cast<ID3D11DeviceContext1&>(cmd);
		cmdQueue.RSSetState(pipeline.rasterizerState);
		cmdQueue.OMSetDepthStencilState(pipeline.depthStencilState, 1);
		// cmdQueue.OMSetBlendState() // TODO
		cmdQueue.RSSetViewports(1, viewport); // TOOD: move to render target setup?

		//cmdQueue.IASetPrimitiveTopology(pipeline.topology);
		//cmdQueue.IASetInputLayout(pipeline.inputLayout);
		//cmdQueue.VSSetShader(pipeline.vertexShader, nullptr, 0);

		//if (pipeline.vertexConstantBuffersCount == 0)
			//cmdQueue.VSSetConstantBuffers(0, ArrayCount(rs->zeroBuffer), rs->zeroBuffer);
		//else
			//cmdQueue.VSSetConstantBuffers(0, pipeline.vertexConstantBuffersCount, (ID3D11Buffer* const*)pipeline.vertexConstantBuffers);
		//cmdQueue.PSSetShader(pipeline.pixelShader, nullptr, 0);
		// cmdQueue->PSSetConstantBuffers(); // TODO
	}

	auto Submit(RendererState& rs, const DXLayer::CmdQueue& cmd, const DescribedMesh& mesh) -> void {
		auto queue = cmd.queue.Get();
		const auto program = mesh.material.program;
		if (program == nullptr) {
			Logger::Error("Mesh material program is null");
			return;
		}

		const auto& gpuData = mesh.gpuData;
		const auto& material = mesh.material;

		material.program->Bind(queue);
		queue->IASetPrimitiveTopology(gpuData->topology);
		DXLayer::SetIndexBuffer(*queue, gpuData->indexBuffer);
		DXLayer::SetVertexBuffer(*queue, gpuData->vertexBuffer.buffer.get(), gpuData->vertexBuffer.stride, gpuData->vertexBuffer.offset);

		for (auto& tex : material.textures) {
			queue->PSSetShaderResources(0, 1, &tex.srv);
			queue->PSSetSamplers(0, 1, &tex.samplerState);
		}
		
		queue->VSSetConstantBuffers(0, ArrayCount(rs.g_d3dConstantBuffers), rs.g_d3dConstantBuffers);
		// c->VSSetConstantBuffers(0, ArrayCount(material.constantBuffer), &material.constantBuffer);

		SetPipelineState(*queue, &rs.g_Viewport, mesh.material.pipelineState);
		DXLayer::DrawIndexed(cmd, mesh.gpuData->indexCount, 0, 0);
	}

	auto DrawModel(RendererState& rs, const Nickel::Renderer::DXLayer::CmdQueue& cmd, const DescribedMesh& mesh, float offsetX = 0.0f, float offsetY = 0.0f) -> void { // TODO: const Material* overrideMat = nullptr
		auto c = cmd.queue.Get();
		Assert(c != nullptr);

		// update:
		const auto t = mesh.transform;
		auto worldMat = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(t.scaleX, t.scaleY, t.scaleZ));
		worldMat *= XMMatrixTranslation(t.positionX+offsetX, t.positionY + offsetY, t.positionZ);
		c->UpdateSubresource1(rs.g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Object], 0, nullptr, &worldMat, 0, 0, 0);

		Submit(rs, cmd, mesh);		
	}

	auto DrawBunny(const Nickel::Renderer::DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		Assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		//SetPipelineState(cmdQueue, rs->g_Viewport, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[0];

		cmdQueue.PSSetShaderResources(0, 1, &rs->matCapTexture.srv);
		cmdQueue.PSSetSamplers(0, 1, &rs->matCapTexture.samplerState);

		Renderer::DXLayer::SetIndexBuffer(cmdQueue, mesh->indexBuffer);
		Renderer::DXLayer::SetVertexBuffer(cmdQueue, mesh->vertexBuffer.buffer.get(), sizeof(VertexPosUV), offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto DrawLight(const DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		Assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		//SetPipelineState(cmdQueue, rs->g_Viewport, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[0];

		cmdQueue.PSSetShaderResources(0, 1, &rs->matCapTexture.srv);
		cmdQueue.PSSetSamplers(0, 1, &rs->matCapTexture.samplerState);

		Renderer::DXLayer::SetIndexBuffer(cmdQueue, mesh->indexBuffer);
		Renderer::DXLayer::SetVertexBuffer(cmdQueue, mesh->vertexBuffer.buffer.get(), sizeof(VertexPosUV), offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto DrawSuzanne(const DXLayer::CmdQueue& cmd, RendererState* rs, PipelineState pipelineState) -> void {
		Assert(cmd.queue != nullptr);
		auto& cmdQueue = *cmd.queue.Get();
		//SetPipelineState(cmdQueue, rs->g_Viewport, pipelineState);

		GPUMeshData* mesh = &rs->GPUMeshData[1];

		cmdQueue.PSSetShaderResources(0, ArrayCount(rs->zeroResourceViews), rs->zeroResourceViews);
		cmdQueue.PSSetSamplers(0, ArrayCount(rs->zeroSamplerStates), rs->zeroSamplerStates);

		Renderer::DXLayer::SetIndexBuffer(cmdQueue, mesh->indexBuffer);

		auto& vertexBuffer = rs->GPUMeshData[1].vertexBuffer;
		Renderer::DXLayer::SetVertexBuffer(cmdQueue, vertexBuffer.buffer.get(), sizeof(VertexPosColor), vertexBuffer.offset);

		cmdQueue.UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

		DXLayer::DrawIndexed(cmd, mesh->indexCount, 0, 0);
	}

	auto GetVertexPosUVFromModelData(MeshData* data) -> std::vector<VertexPosUV> {
		Assert(data != nullptr);

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
		Assert(data != nullptr);

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

	void CreatePass() {

	}

	auto Initialize(GameMemory* memory, RendererState* rs) -> void {
		Assert(memory != nullptr);
		Assert(rs != nullptr);
		Assert(rs->device);
		Assert(rs->cmdQueue.queue);

		ID3D11Device1* device = rs->device.Get();
		ResourceManager::Init(device);

		if (!LoadContent(rs))
			Logger::Error("Content couldn't be loaded");

		rs->defaultDepthStencilBuffer = DXLayer::CreateDepthStencilTexture(device, rs->backbufferWidth, rs->backbufferHeight);
		Assert(rs->defaultDepthStencilBuffer != nullptr);
		rs->defaultDepthStencilView = DXLayer::CreateDepthStencilView(device, rs->defaultDepthStencilBuffer);

		auto defaultDepthStencilState = DXLayer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
		auto defaultRasterizerState = DXLayer::CreateDefaultRasterizerState(device);

		D3D11_RENDER_TARGET_BLEND_DESC1 blendDescription = { 0 };
		D3D11_BLEND_DESC1 stateDesc = { 0 };
		stateDesc.AlphaToCoverageEnable = false;
		stateDesc.IndependentBlendEnable = true;
		for (int i = 0; i < ArrayCount(stateDesc.RenderTarget); i++) {
			stateDesc.RenderTarget[i] = blendDescription;
		}

		// TODO: this is only dummy test code
		ID3D11BlendState1* state;
		device->CreateBlendState1(&stateDesc, &state);
		const FLOAT blendFactor[4] = { 0 };
		// rs->cmdQueue.queue->OMSetBlendState(state, blendFactor, 0);

		//pip->depthStencilState = depthStencilState;
		//pip->rasterizerState = rasterizerState;
		//pip->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		//pip->inputLayout = DXLayer::CreateInputLayout(device, vertexPosUVLayoutDesc, std::span{g_TexVertexShader});
		//pip->vertexShader = rs->textureMat.program->vertexShader;
		//pip->vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
		//pip->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
		//pip->pixelShader = rs->textureMat.program->pixelShader;

		// ApplyPipeline(device, &pip); // TODO

		// simple pip:
		//pip2->depthStencilState = depthStencilState;
		//pip2->rasterizerState = rasterizerState;
		//pip2->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		//pip2->inputLayout = DXLayer::CreateInputLayout(device, vertexPosColorLayoutDesc, std::span{g_SimpleVertexShader});

		//pip2->vertexShader = rs->simpleMat.program->vertexShader;
		//pip2->vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
		//pip2->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
		//pip2->pixelShader = rs->simpleMat.program->pixelShader;


		// pip->vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
		// pip->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);

		// materials
		{
			auto& simpleMat = rs->simpleMat;
			simpleMat = Material{
				.program = &rs->simpleProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				},
			};
		}
		
		
		// simpleMat.constantBuffer = DXLayer::CreateConstantBuffer(device, )
		{
			auto& textureMat = rs->textureMat;
			textureMat = Material{
				.program = &rs->textureProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				}
			};
			textureMat.textures = std::vector<TextureDX11>(1);
			textureMat.textures[0] = rs->matCapTexture;
		}
		
		{
			auto rasterizerDesc = DXLayer::GetDefaultRasterizerDescription();
			rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
			auto& skyboxMat = rs->backgroundMat;
			skyboxMat = Material{
				.program = &rs->backgroundProgram,
				.pipelineState = PipelineState{
					.rasterizerState = DXLayer::CreateRasterizerState(device, rasterizerDesc),
					.depthStencilState = defaultDepthStencilState
				}
			};
			//textureMat.textures = std::vector<TextureDX11>(1);
			//textureMat.textures[0] = rs->matCapTexture;
		}
		

		rs->bunny.material = rs->textureMat;
	}

	const FLOAT clearColor[4] = { 0.13333f, 0.13333f, 0.13333f, 1.0f };

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

		auto cmd = rs->cmdQueue;
		auto& queue = *cmd.queue.Get();

		queue.UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Frame], 0, nullptr, &frameData, 0, 0, 0);

		// RENDER ---------------------------
		Assert(rs->defaultRenderTargetView != nullptr);
		Assert(rs->defaultDepthStencilView != nullptr);
		DXLayer::SetRenderTarget(queue, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
		queue.RSSetViewports(1, &rs->g_Viewport);

		DXLayer::ClearFlag clearFlag = DXLayer::ClearFlag::CLEAR_COLOR | DXLayer::ClearFlag::CLEAR_DEPTH;
		DXLayer::Clear(rs->cmdQueue, static_cast<u32>(clearFlag), rs->defaultRenderTargetView, rs->defaultDepthStencilView, clearColor, 1.0f, 0);

		
		/*
		DescribedMesh meshes[3];
		for (int i = 0; i < 5; i++) { // rs->sceneMeshes
			auto mesh = meshes[i];
			auto material = mesh.material;

			if (material.HasProperty("u_Color"))
				material.SetProperty("u_Color", clearColor);

			if (currentProgram != material.program) {
				currentProgram = material.program;
				// SetProgramData(currentProgram); Bind
			}

			// bind vertex and index buffer
			// bind textures, samplers
			// set 

			// draw
		}
		*/

		//std::vector<ID3D11ShaderResourceView*> nullSRVs(gbuffer.size() + 1, nullptr); // TODO
		//context->PSSetShaderResources(0, static_cast<u32>(nullSRVs.size()), nullSRVs.data());

		for (int y = -20; y < 20; y++) {
			for (int x = -20; x < 20; x++) {
				DrawModel(*rs, cmd, rs->bunny, x*2.0f, y*2.0f);
			}
		}

		// DrawBunny(cmd, rs, rs->pipelineStates[0]);

		// rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(2.0f, 2.0f, 2.0f));
		// rs->g_WorldMatrix *= XMMatrixTranslation(lightPos.x, lightPos.y, lightPos.z);
		//DrawLight(cmd, rs, rs->pipelineStates[0]);

		// rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.5f, 0.5f, 0.5f));
		// rs->g_WorldMatrix *= XMMatrixTranslation(radius * cos(XMConvertToRadians(180.0f)), 0.0f, radius * sin(XMConvertToRadians(180.0f)));
		// DrawSuzanne(cmd, rs, rs->pipelineStates[1]);
		

		rs->swapChain->Present(1, 0);
	}

	auto LoadContent(RendererState* rs) -> bool {
		Assert(rs != nullptr);
		Assert(rs->device != nullptr);
		Assert(rs->cmdQueue.queue != nullptr);

		auto device = rs->device.Get();

		{ // Bunny
			LoadMeshAndSetup("Data/Models/bny.obj");

			MeshData meshData;
			LoadBunnyMesh(meshData);

			std::vector<VertexPosUV> vertexFormatData = GetVertexPosUVFromModelData(&meshData);
			auto vertexCount = static_cast<u32>(vertexFormatData.size());
			auto indexCount = static_cast<u32>(meshData.i.size());

			auto vertexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexFormatData.data() };
			auto indexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = meshData.i.data() };
			auto indexByteWidthSize = sizeof(meshData.i[0]) * indexCount;

			//auto vertexBuffer = VertexBuffer{
				//.buffer = Renderer::DXLayer::CreateVertexBuffer(device, (sizeof(vertexFormatData[0]) * vertexCount), &vertexSubresource),
				//.stride = sizeof(VertexPosUV),
				//.offset = 0
			//};

			auto& gpuMeshData = rs->GPUMeshData[0];	
			gpuMeshData = GPUMeshData{
				.vertexCount = vertexCount,

				.indexBuffer = Renderer::DXLayer::CreateIndexBuffer(device, indexByteWidthSize, &indexSubresource),
				.indexCount = indexCount,
				.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
			};
			gpuMeshData.vertexBuffer.Create<VertexPosUV>(device, std::span(vertexFormatData), false);
			
			auto& bunny = rs->bunny;
			bunny.transform.scaleX = 12.0f;
			bunny.transform.scaleY = 12.0f;
			bunny.transform.scaleZ = 12.0f;
			bunny.transform.positionX = 0.0f;
			bunny.transform.positionY = -2.0f;
			bunny.transform.positionZ = 0.0f;
			bunny.gpuData = &rs->GPUMeshData[0];
			bunny.mesh = meshData;
		}

		{ // Suzanne
			MeshData meshData;
			LoadSuzanneModel(meshData);

			std::vector<VertexPosColor> vertexFormatData = GetVertexPosColorFromModelData(&meshData); // TODO: change this to GetVertexPosUVFromModelData and make easy to debug
			auto vertexCount = static_cast<u32>(vertexFormatData.size());
			auto indexCount = static_cast<u32>(meshData.i.size());

			auto vertexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = vertexFormatData.data() };
			auto indexSubresource = D3D11_SUBRESOURCE_DATA{ .pSysMem = meshData.i.data() };
			auto indexByteWidthSize = sizeof(meshData.i[0]) * indexCount;

			auto& gpuMeshData = rs->GPUMeshData[1];
			gpuMeshData = GPUMeshData{
				.vertexCount = vertexCount,

				.indexBuffer = DXLayer::CreateIndexBuffer(device, indexByteWidthSize, &indexSubresource),
				.indexCount = indexCount
			};
			gpuMeshData.vertexBuffer.Create<VertexPosColor>(device, std::span(vertexFormatData), false);
		}

		// Create the constant buffers for the variables defined in the vertex shader.
		rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Appliation] = DXLayer::CreateConstantBuffer(device, sizeof(XMMATRIX));
		rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Object]     = DXLayer::CreateConstantBuffer(device, sizeof(XMMATRIX));
		rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Frame]      = DXLayer::CreateConstantBuffer(device, sizeof(PerFrameBufferData));

		// Create shader programs
		rs->simpleProgram.Create(rs->device.Get(), std::span{g_SimpleVertexShader}, std::span{g_SimplePixelShader});
		rs->textureProgram.Create(rs->device.Get(), std::span{g_TexVertexShader}, std::span{g_TexPixelShader});
		rs->backgroundProgram.Create(rs->device.Get(), std::span{ g_SimpleVertexShader }, std::span{ g_BackgroundPixelShader });

		rs->matCapTexture = ResourceManager::LoadTexture(L"Data/Textures/matcap.jpg");

		// Setup the projection matrix.
		RECT clientRect;
		GetClientRect(rs->g_WindowHandle, &clientRect);

		// Compute the exact client dimensions.
		// This is required for a correct projection matrix.
		float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
		float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

		rs->g_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

		rs->cmdQueue.queue->UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBuffer::CB_Appliation], 0, nullptr, &rs->g_ProjectionMatrix, 0, 0, 0);

		CreateCubeMap(*rs, 256, 256);

		return true;
	}

	auto SetDefaultPass(const DXLayer::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void {
		Assert(cmd.queue != nullptr);
		cmd.queue->OMSetRenderTargets(1, renderTargetView, &depthStencilView);
	}
}