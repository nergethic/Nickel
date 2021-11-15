#include "game.h"
#include "renderer.h"

namespace Nickel {
	using namespace Renderer;

	auto SetPipelineState(const ID3D11DeviceContext1& cmd, D3D11_VIEWPORT* viewport, const PipelineState& pipeline) -> void {
		ID3D11DeviceContext1& cmdQueue = const_cast<ID3D11DeviceContext1&>(cmd);
		cmdQueue.RSSetState(pipeline.rasterizerState);
		cmdQueue.OMSetDepthStencilState(pipeline.depthStencilState, 1);
		// cmdQueue.OMSetBlendState() // TODO
		cmdQueue.RSSetViewports(1, viewport); // TOOD: move to render target setup?
	}

	auto Submit(RendererState& rs, const DXLayer::CmdQueue& cmdQueue, const DescribedMesh& mesh) -> void {
		auto cmd = cmdQueue.queue.Get();
		const auto program = mesh.material.program;
		if (program == nullptr) {
			Logger::Error("Mesh material program is null");
			return;
		}

		const auto& gpuData = mesh.gpuData;
		auto& mat = mesh.material;

		if (mesh.gpuData.indexCount == 0) {
			Logger::Warn("Index count is 0!");
			return;
		}

		mat.program->Bind(cmd);
		cmd->IASetPrimitiveTopology(gpuData.topology);

		const auto indexBuffer = gpuData.indexBuffer.buffer.get();
		const auto vertexBuffer = gpuData.vertexBuffer.buffer.get();

		DXLayer::SetIndexBuffer(*cmd, indexBuffer);
		DXLayer::SetVertexBuffer(*cmd, vertexBuffer, gpuData.vertexBuffer.stride, gpuData.vertexBuffer.offset);

		if (mat.textures.size() > 0) {
			cmd->PSSetSamplers(0, 1, &mat.textures[0].samplerState);
			for (int i = 0; i < mat.textures.size(); i++) {
				const auto& tex = mat.textures[i];
				cmd->PSSetShaderResources(i, 1, &tex.srv); // TODO: this just puts every texture in slot 0 - fix it
			}
		}
		
		cmd->VSSetConstantBuffers(0, ArrayCount(rs.g_d3dConstantBuffers), rs.g_d3dConstantBuffers);
		cmd->PSSetConstantBuffers(0, ArrayCount(rs.g_d3dConstantBuffers), rs.g_d3dConstantBuffers);

		const auto vertexConstantBuffer = mat.vertexConstantBuffer.buffer.Get();
		if (vertexConstantBuffer != nullptr) {
			Assert(mat.vertexConstantBuffer.index < D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT)
			cmd->VSSetConstantBuffers(mat.vertexConstantBuffer.index, 1, &vertexConstantBuffer);
		}

		const auto pixelConstantBuffer = mat.pixelConstantBuffer.buffer.Get();
		if (pixelConstantBuffer != nullptr) {
			Assert(mat.pixelConstantBuffer.index < D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT)
			cmd->PSSetConstantBuffers(mat.pixelConstantBuffer.index, 1, &pixelConstantBuffer);
		}

		SetPipelineState(*cmd, &rs.g_Viewport, mesh.material.pipelineState);
		DXLayer::DrawIndexed(cmdQueue, mesh.gpuData.indexCount, 0, 0);
	}

	auto DrawModel(RendererState& rs, const Nickel::Renderer::DXLayer::CmdQueue& cmd, const DescribedMesh& mesh, Vec3 offset = {0.0, 0.0, 0.0}) -> void { // TODO: const Material* overrideMat = nullptr
		auto c = cmd.queue.Get();
		Assert(c != nullptr);

		// update:
		const auto t = mesh.transform;
		const auto& pos = t.position + offset;
		auto worldMat = XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z)
			* XMMatrixRotationRollPitchYawFromVector(FXMVECTOR{ t.rotation.x, t.rotation.y, t.rotation.z })
			* XMMatrixTranslation(pos.x, pos.y, pos.z);

		const auto& viewProjectionMatrix = rs.mainCamera.get()->GetViewProjectionMatrix();

		PerObjectBufferData data;
		data.modelMatrix = XMMatrixTranspose(worldMat);
		data.viewProjectionMatrix = XMMatrixTranspose(viewProjectionMatrix);
		data.modelViewProjectionMatrix = XMMatrixTranspose(worldMat * viewProjectionMatrix);

		c->UpdateSubresource1(rs.g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Object], 0, nullptr, &data, 0, 0, 0);

		Submit(rs, cmd, mesh);		
	}

	auto GetVertexPosUVFromModelData(MeshData* data) -> std::vector<VertexPosUV> {
		Assert(data != nullptr);

		std::vector<VertexPosUV> result;
		f32 x, y, z;
		for (u32 i = 0; i < data->v.size(); ++i) {
			const auto& pos    = data->v[i].position;
			const auto& normal = data->v[i].normal;
			const auto& uv     = data->v[i].uv[0];

			VertexPosUV vertexData = {
				XMFLOAT3(-pos.x, pos.y, -pos.z),
				XMFLOAT3(-normal.x, normal.y, -normal.z),
				//XMFLOAT3(clamp(0.0f, 1.0f, x), clamp(0.0f, 1.0f, y), clamp(0.0f, 1.0f, z))
				// XMFLOAT3(0.53333, 0.84705, 0.69019),
				XMFLOAT2(uv.x, uv.y)
			};

			result.push_back(vertexData);
		}

		return result;
	}

	auto GetVertexPosColorFromModelData(MeshData* data) -> std::vector<VertexPosColor> {
		Assert(data != nullptr);

		std::vector<VertexPosColor> result;
		f32 x, y, z;
		for (u32 i = 0; i < data->v.size(); ++i) {
			const auto& pos = data->v[i].position;
			const auto& normal = data->v[i].normal;

			VertexPosColor vertexData = {
				XMFLOAT3(-pos.x, pos.y, -pos.z),
				XMFLOAT3(-normal.x, normal.y, -normal.z),
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
	}

	void LoadMeshAndSetup(const std::string& path) {
		MeshData meshData;
		LoadObjMeshData(meshData, path);
	}

	auto Initialize(GameMemory* memory, RendererState* rs) -> void {
		Assert(memory != nullptr);
		Assert(rs != nullptr);
		Assert(rs->device);
		Assert(rs->cmdQueue.queue);

		ID3D11Device1* device = rs->device.Get();
		auto resourceManager = ResourceManager::GetInstance();
		resourceManager->Init(device);

		rs->mainCamera = std::make_unique<Camera>(45.0f, 1.5f, 0.1f, 100.0f);

		background.Create(device);

		rs->defaultDepthStencilBuffer = DXLayer::CreateDepthStencilTexture(device, rs->backbufferWidth, rs->backbufferHeight);
		Assert(rs->defaultDepthStencilBuffer != nullptr);
		rs->defaultDepthStencilView = DXLayer::CreateDepthStencilView(device, rs->defaultDepthStencilBuffer);

		// Create the constant buffers for the variables defined in the vertex shader.
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Appliation] = DXLayer::CreateConstantBuffer(device, sizeof(PerApplicationData));
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Object] = DXLayer::CreateConstantBuffer(device, sizeof(PerObjectBufferData));
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Frame] = DXLayer::CreateConstantBuffer(device, sizeof(PerFrameBufferData));

		// Create shader programs
		rs->pbrProgram.Create(rs->device.Get(), std::span{ g_PbrVertexShader }, std::span{ g_PbrPixelShader });
		rs->lineProgram.Create(rs->device.Get(), std::span{ g_LineVertexShader }, std::span{ g_ColorPixelShader });
		rs->simpleProgram.Create(rs->device.Get(), std::span{ g_SimpleVertexShader }, std::span{ g_SimplePixelShader });
		rs->textureProgram.Create(rs->device.Get(), std::span{ g_TexVertexShader }, std::span{ g_TexPixelShader });

		rs->albedoTexture = resourceManager->LoadTexture(L"Data/Models/DamagedHelmet/Default_albedo.jpg");
		rs->normalTexture = resourceManager->LoadTexture(L"Data/Models/DamagedHelmet/Default_normal.jpg");
		rs->aoTexture = resourceManager->LoadTexture(L"Data/Models/DamagedHelmet/Default_AO.jpg");
		rs->metalRoughnessTexture = resourceManager->LoadTexture(L"Data/Models/DamagedHelmet/Default_metalRoughness.jpg");
		rs->emissiveTexture = resourceManager->LoadTexture(L"Data/Models/DamagedHelmet/Default_emissive.jpg");
		rs->debugBoxTexture = resourceManager->LoadTexture(L"Data/Models/BoxTextured/CesiumLogoFlat.png");

		rs->matCapTexture = resourceManager->LoadTexture(L"Data/Textures/matcap.jpg");

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
			textureMat.textures = std::vector<DXLayer::TextureDX11>(1);
			textureMat.textures[0] = rs->albedoTexture;
		}

		{ // PBR mat
			auto& pbrMat = rs->pbrMat;
			pbrMat = Material{
				.program = &rs->pbrProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				},
				.pixelConstantBuffer = {
					.buffer = DXLayer::CreateConstantBuffer(device, sizeof(PbrPixelBufferData)),
					.index = 3
				}
			};
			pbrMat.textures = std::vector<DXLayer::TextureDX11>(5);
			pbrMat.textures[0] = rs->albedoTexture;
			pbrMat.textures[1] = rs->normalTexture;
			pbrMat.textures[2] = rs->metalRoughnessTexture;
			pbrMat.textures[3] = rs->aoTexture;

			PbrPixelBufferData bufferData{
				.lightPositions = {XMFLOAT4(0.0f, 0.0f, 0.0f, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 0)},
				.lightColors = {XMFLOAT4(0.1f, 0.1f, 0.6f, 0), XMFLOAT4(1.0f, 0.0f, 0.0f, 0), XMFLOAT4(0.0f, 1.0f, 0.0f, 0), XMFLOAT4(0.4f, 0.4f, 0.4f, 0)},
				.albedoFactor = XMFLOAT4(0.2f, 0.05f, 0.75f, 0.0f),
				.metallic = 0.0f,
				.roughness = 0.4f,
				.ao = 0.5f
			};
			pbrMat.pixelConstantBuffer.Update(rs->cmdQueue.queue.Get(), bufferData);
		}

		if (!LoadContent(rs))
			Logger::Error("Content couldn't be loaded");

		// set projection matrix
		RECT clientRect;
		GetClientRect(rs->g_WindowHandle, &clientRect);

		float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
		float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

		const f32 aspectRatio = static_cast<f32>(clientWidth) / clientHeight;
		const auto& projectionMatrix = rs->mainCamera.get()->GetProjectionMatrix();
		auto data = PerApplicationData{
			.projectionMatrix = XMMatrixTranspose(projectionMatrix),
			.clientData = XMFLOAT3(clientWidth,clientHeight,aspectRatio)
		};

		rs->cmdQueue.queue->UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Appliation], 0, nullptr, &data, 0, 0, 0);
	}

	static f32 previousMouseX = 0.5f;
	static f32 previousMouseY = 0.5f;
	static XMFLOAT4 light1Pos = { 0.0, 0.0, 0.0, 0.0 };
	static XMFLOAT4 light2Pos = { 0.0, 0.0, 0.0, 0.0 };
	static XMFLOAT4 light3Pos = { 0.0, 0.0, 0.0, 0.0 };
	static XMFLOAT4 light4Pos = { 0.0, 0.0, 0.0, 0.0 };
	static f32 timer = 0.0f;
	const FLOAT clearColor[4] = { 0.13333f, 0.13333f, 0.13333f, 1.0f };
	auto UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void {
		// GameState* gs = (GameState*)memory;

		timer += 0.01f;
		if (timer > 1000.0f)
			timer -= 1000.0f;

		static float angle = -90.0f;
		float radius = 2.1f;
		angle += 1.2f;
		XMFLOAT3 lightPos = XMFLOAT3(radius * cos(XMConvertToRadians(angle)), 0.0f, radius * sin(XMConvertToRadians(angle)));

		auto& camera = *rs->mainCamera.get();
		PerFrameBufferData frameData;
		frameData.viewMatrix = XMMatrixTranspose(camera.GetViewMatrix());
		frameData.cameraPosition = XMFLOAT3(camera.position.x, camera.position.y, camera.position.z);
		frameData.lightPosition = lightPos;

		auto& cmd = rs->cmdQueue;
		auto& queue = *cmd.queue.Get();

		queue.UpdateSubresource1(rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Frame], 0, nullptr, &frameData, 0, 0, 0);

		//light1Pos = XMFLOAT4(3.0f*cos(timer), 3.0f*sin(timer), 0.0, 0.0);
		Vec3 newLightPos = { camera.position.x, camera.position.y-2.0f, static_cast<f32>(-4.0f + sin(timer) * 5.0f) };
		light4Pos = XMFLOAT4(newLightPos.x, newLightPos.y, newLightPos.z, 0.0f);
		rs->debugCube.transform.position = newLightPos;

		PbrPixelBufferData bufferData{
			.lightPositions = {light1Pos, light2Pos, light3Pos, light4Pos},
			.lightColors = {XMFLOAT4(200.0f, 200.0f, 200.0f, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 0), XMFLOAT4(300.0f, 300.0f, 300.0f, 0)},
			.albedoFactor = XMFLOAT4(0.2f, 0.05f, 0.75f, 0.0f),
			.metallic = 0.6f,
			.roughness = 0.4f,
			.ao = 1.0f
		};
		rs->pbrMat.pixelConstantBuffer.Update(rs->cmdQueue.queue.Get(), bufferData);

		// RENDER ---------------------------
		Assert(rs->defaultRenderTargetView != nullptr);
		Assert(rs->defaultDepthStencilView != nullptr);
		DXLayer::SetRenderTarget(queue, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
		queue.RSSetViewports(1, &rs->g_Viewport);

		DXLayer::ClearFlag clearFlag = DXLayer::ClearFlag::CLEAR_COLOR | DXLayer::ClearFlag::CLEAR_DEPTH;
		DXLayer::Clear(rs->cmdQueue, static_cast<u32>(clearFlag), rs->defaultRenderTargetView, rs->defaultDepthStencilView, clearColor, 1.0f, 0);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Hello imgui    ");
		ImGui::Text("Lorem Ipsum     ");
		ImGui::End();
		
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

		// rs->bunny.transform.rotation.y += 0.005;
		// rs->skybox.transform.rotation.y += 0.0005;

		for (auto& line : rs->lines)
			if (line.material.program != nullptr)
				DrawModel(*rs, cmd, line);

		for (int y = -2; y <= 2; y++) {
			for (int x = -2; x <= 2; x++) {
				for (int i = 0; i < rs->bunny.size(); i++) {
					const auto t = rs->bunny[i].transform;
					rs->bunny[i].transform.rotation.y += 0.0004f;
					DrawModel(*rs, cmd, rs->bunny[i], {x * 3.0f, y * 3.0f, -4.0f});
				}
			}
		}

		DrawModel(*rs, cmd, rs->debugCube);
		// DrawModel(*rs, cmd, rs->debugBoxTextured);

		f32 dtMouseX = input->normalizedMouseX - previousMouseX;
		f32 dtMouseY = input->normalizedMouseY - previousMouseY;
		camera.lookAtPosition.x += dtMouseX * 50.0f;
		camera.lookAtPosition.y += dtMouseY * 50.0f;
		camera.RecalculateMatrices();

		DrawModel(*rs, cmd, background.skyboxMesh);

		// DrawBunny(cmd, rs, rs->pipelineStates[0]);

		// rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(2.0f, 2.0f, 2.0f));
		// rs->g_WorldMatrix *= XMMatrixTranslation(lightPos.x, lightPos.y, lightPos.z);
		//DrawLight(cmd, rs, rs->pipelineStates[0]);

		// rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.5f, 0.5f, 0.5f));
		// rs->g_WorldMatrix *= XMMatrixTranslation(radius * cos(XMConvertToRadians(180.0f)), 0.0f, radius * sin(XMConvertToRadians(180.0f)));
		// DrawSuzanne(cmd, rs, rs->pipelineStates[1]);
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		rs->swapChain->Present(1, 0);

		previousMouseX = input->normalizedMouseX;
		previousMouseY = input->normalizedMouseY;
	}

	auto CreateLineIndices(u32 length) -> std::vector<u32> {
		auto indices = std::vector<u32>(length*6);

		u32 c = 0;
		u32 index = 0;

		for (u32 j = 0; j < length; j++) {
			auto i = index;
			indices[c++] = i + 0;
			indices[c++] = i + 1;
			indices[c++] = i + 2;
			indices[c++] = i + 2;
			indices[c++] = i + 1;
			indices[c++] = i + 3;
			index += 2;
		}

		return indices;
	}

	auto DuplicateLineVertices(std::span<Vec3> vertices, bool mirror = false) -> std::vector<Vec3> {
		Assert(vertices.data() != nullptr && vertices.size() >= 0);
		auto result = std::vector<Vec3>();
		for (const auto& v : vertices) {
			const auto outV = mirror ? Vec3(-v.x, -v.y, -v.z) : v;
			result.push_back(outV);
			result.push_back(v);
		}
		return result;
	}

	auto DuplicateLineVertices(std::span<i32> vertices, bool mirror = false) -> std::vector<i32> {
		Assert(vertices.data() != nullptr && vertices.size() >= 0);
		auto result = std::vector<i32>();
		for (const auto& v : vertices) {
			const auto outV = mirror ? -v : v;
			result.push_back(outV);
			result.push_back(v);
		}
		return result;
	}

	auto Clamp(u32 val, u32 min, u32 max) {
		if (val < min)
			val = min;
		else if (val > max)
			val = max;

		return val;
	}

	/*
	auto GenerateLineInDir(Vec3 startPos, Vec3 pointOffset, Vec3 dir, u32 pointCount) -> std::vector<Vec3> {
		auto result = std::vector<Vec3>(pointCount);
		const f32 len = std::sqrt(pointOffset.x * pointOffset.x + pointOffset.y * pointOffset.y + pointOffset.z * pointOffset.z);
		const auto dirNormalized = Vec3{
			pointOffset.x / len,
			pointOffset.y / len,
			pointOffset.z / len,
		};
		for (u32 i = 0; i < pointCount; i++) {
			auto val = Vec3{ startPos.x + (dirNormalized.x*pointOffset.x*i) , startPos.y + (dirNormalized.y*pointOffset.y*i), startPos.z + (dirNormalized.z*pointOffset.z*i) };
			result[i] = val;
		}

		return result;
	}
	*/

	auto GenerateLine(RendererState* rs, std::vector<Vec3> vertexData, DescribedMesh& describedMesh) -> void {
		auto device = rs->device.Get();

		//each pair has a mirrored direction 
		auto dirs = std::vector<i32>(vertexData.size());
		for (int i = 0; i < vertexData.size(); i++) {
			dirs[i] = 1;
		}
		auto direction = DuplicateLineVertices(std::span{ dirs }, true);
		auto positions = DuplicateLineVertices(std::span{ vertexData });

		auto prevs = std::vector<Vec3>(vertexData.size());
		for (u32 i = 0; i < vertexData.size(); i++) {
			u32 idx = Clamp(i - 1, 0, prevs.size() - 1);
			prevs[i] = vertexData[idx];
		}
		auto previousVertex = DuplicateLineVertices(std::span{ prevs });

		auto nexts = std::vector<Vec3>(vertexData.size());
		for (u32 i = 0; i < vertexData.size(); i++) {
			u32 idx = Clamp(i + 1, 0, nexts.size() - 1);
			nexts[i] = vertexData[idx];
		}
		auto nextVertex = DuplicateLineVertices(std::span{ nexts });

		//
		auto vertexFormatData = std::vector<LineVertexData>();
		for (int i = 0; i < direction.size(); i++) {
			auto vertex = LineVertexData{
				.position = XMFLOAT3(positions[i].x, positions[i].y, positions[i].z),
				.previous = XMFLOAT3(previousVertex[i].x, previousVertex[i].y, previousVertex[i].z),
				.next = XMFLOAT3(nextVertex[i].x, nextVertex[i].y, nextVertex[i].z),
				.direction = XMFLOAT3(direction[i], direction[i], direction[i])
			};
			vertexFormatData.push_back(vertex);
		}

		auto indexData = CreateLineIndices(vertexData.size());

		const u32 indexCount = indexData.size();
		const u32 vertexCount = vertexFormatData.size() - 6;

		describedMesh.transform.scale = {1.0f, 1.0f, 1.0f};
		describedMesh.transform.position = { 0.0f, 0.0f, 0.0f };
		describedMesh.gpuData = GPUMeshData{
			.vertexCount = vertexCount,
			.indexCount = indexCount,
			.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		};
		describedMesh.gpuData.indexBuffer.Create(device, std::span(indexData));
		describedMesh.gpuData.vertexBuffer.Create<LineVertexData>(device, std::span(vertexFormatData), false);
		describedMesh.material = rs->lineMat;
		//line.mesh = meshData; // TODO: is this useless?
	}

	auto GenerateLinePoints(Vec3 from, Vec3 to, u32 pointCount) -> std::vector<Vec3> {
		auto result = std::vector<Vec3>();
		for (i32 i = 0; i <= pointCount; i++) {
			result.push_back(Vec3::Lerp(from, to, static_cast<f32>(i) / pointCount));
		}

		return result;
	}

	auto LoadContent(RendererState* rs) -> bool {
		Assert(rs != nullptr);
		Assert(rs->device != nullptr);
		Assert(rs->cmdQueue.queue != nullptr);

		auto device = rs->device.Get();
		auto resourceManager = ResourceManager::GetInstance();

		{ // convert vertex data to be LineVertexData compatible
			auto defaultDepthStencilState = DXLayer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
			auto rasterizerDesc = DXLayer::GetDefaultRasterizerDescription();
			rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
			//rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
			auto defaultRasterizerState = DXLayer::CreateRasterizerState(device, rasterizerDesc);
			auto& lineMat = rs->lineMat;
			lineMat = Material{
				.program = &rs->lineProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				},
				.vertexConstantBuffer = {
					.buffer = DXLayer::CreateConstantBuffer(device, sizeof(LineBufferData)),
					.index = 3
				}
			};
			LineBufferData bufferData{
				.thickness = 0.04f,
				.miter = 0
			};
			lineMat.vertexConstantBuffer.Update(rs->cmdQueue.queue.Get(), bufferData);

			const auto pointOffset = Vec3{ 0.5f, 0.0f, 0.5f };
			// auto line1 = GenerateLineInDir(Vec3{ 0.0, 0.0, 0.0 }, pointOffset, Vec3{ 0.0, 0.0, 1.0 }, 10);

			u32 lineIdx = 0;
			const u32 linePointsCount = 100;
			for (i32 x = -5; x <= 5; x++, lineIdx++) {
				const auto xOffset = static_cast<f32>(x) * 2.0f;
				GenerateLine(rs, GenerateLinePoints(Vec3{ xOffset, 0.0, -10.0 }, Vec3{ xOffset, 0.0, 10.0 }, linePointsCount), rs->lines[lineIdx]);
			}
				
			for (i32 y = -5; y <= 5; y++, lineIdx++) {
				const auto yOffset = static_cast<f32>(y) * 2.0f;
				GenerateLine(rs, GenerateLinePoints(Vec3{ -10.0, 0.0, yOffset }, Vec3{ 10.0, 0.0, yOffset }, linePointsCount), rs->lines[lineIdx]);
			}
		}

		{ // Bunny
			//LoadMeshAndSetup("Data/Models/bny.obj");

			//MeshData meshData;
			// LoadBunnyMesh(meshData);
			//const auto& meshData = *resourceManager->LoadModel("Data/Models/backpack/backpack.obj");
			const auto& meshData = *resourceManager->LoadModel("Data/Models/DamagedHelmet/DamagedHelmet.gltf");
			auto& bunny = rs->bunny;
			bunny = std::vector<DescribedMesh>(meshData.size());
			for (int i = 0; i < meshData.size(); i++) {
				const auto& submesh = meshData[i];

				const u64 vertexCount = submesh.v.size();
				const u64 indexCount = submesh.i.size();

				auto vertexFormatData = std::vector<VertexPosUV>(vertexCount);
				{
					for (int i = 0; i < vertexCount; i++) {
						const auto& v = submesh.v[i];
						vertexFormatData[i] = VertexPosUV{
							.Position = XMFLOAT3(v.position.x, v.position.y, v.position.z),
							.Normal = XMFLOAT3(v.normal.x, v.normal.y, v.normal.z),
							.UV = XMFLOAT2(v.uv[0].x, v.uv[0].y)
						};
					}
				}

				bunny[i] = DescribedMesh{
					.transform = {
						.position = { 1.0f, 1.0f, 1.0f },
						.scale = {1.1f, 1.1f, 1.1f},
						.rotation = {XMConvertToRadians(-60.0f), 0.0f, 0.0f}
					},
					.mesh = submesh, // TODO: is this useless?
					.gpuData = GPUMeshData{
						.vertexCount = vertexCount,
						.indexCount = indexCount,
						.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
					},
					.material = rs->pbrMat
				};

				auto x = submesh.i;
				bunny[i].gpuData.indexBuffer.Create(device, std::span(x));
				bunny[i].gpuData.vertexBuffer.Create<VertexPosUV>(device, std::span(vertexFormatData), false);
			}
		}
		
		{ // Suzanne
			MeshData meshData;
			LoadSuzanneModel(meshData);

			std::vector<VertexPosColor> vertexFormatData = GetVertexPosColorFromModelData(&meshData); // TODO: change this to GetVertexPosUVFromModelData and make easy to debug
			const auto vertexCount = static_cast<u32>(vertexFormatData.size());
			const auto indexCount = static_cast<u32>(meshData.i.size());

			//auto& gpuMeshData = rs->gpuMeshData[1];
			//gpuMeshData = GPUMeshData{
				//.vertexCount = vertexCount,
				//.indexCount = indexCount
			//};
			//gpuMeshData.indexBuffer.Create(device, std::span(meshData.i));
			//gpuMeshData.vertexBuffer.Create<VertexPosColor>(device, std::span(vertexFormatData), false);
		}

		{ // Test Cube
			const float side = 0.5f;
			auto vertexData = std::vector<VertexPosColor>(8);
			auto color = XMFLOAT3{ 0.1803f, 0.3255f, 0.7882f };
			vertexData[0] = VertexPosColor{ .Position = XMFLOAT3(-side,-side,-side), .Normal = {1.0, 0.0, 0.0},  .Color = color };
			vertexData[1] = VertexPosColor{ .Position = XMFLOAT3(side,-side,-side),  .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[2] = VertexPosColor{ .Position = XMFLOAT3(-side,side,-side),  .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[3] = VertexPosColor{ .Position = XMFLOAT3(side,side,-side),   .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[4] = VertexPosColor{ .Position = XMFLOAT3(-side,-side,side),  .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[5] = VertexPosColor{ .Position = XMFLOAT3(side,-side,side),   .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[6] = VertexPosColor{ .Position = XMFLOAT3(-side,side,side),   .Normal = {1.0, 0.0, 0.0 }, .Color = color };
			vertexData[7] = VertexPosColor{ .Position = XMFLOAT3(side,side,side),    .Normal = {1.0, 0.0, 0.0 }, .Color = color };

			auto indices = std::vector<u32>{
				0,2,1, 2,3,1,
				1,3,5, 3,7,5,
				2,6,3, 3,6,7,
				4,5,7, 4,7,6,
				0,4,2, 2,4,6,
				0,1,4, 1,5,4
			};

			const auto vertexCount = static_cast<u32>(vertexData.size());
			const auto indexCount = static_cast<u32>(indices.size());

			auto& cube = rs->debugCube;
			cube.transform.scale = { 0.2f, 0.2f, 0.2f };
			cube.transform.position = { 0.0f, 0.0f, 0.0f };
			cube.gpuData = GPUMeshData{
				.vertexCount = vertexCount,
				.indexCount = indexCount
			};
			cube.gpuData.indexBuffer.Create(device, std::span(indices));
			cube.gpuData.vertexBuffer.Create<VertexPosColor>(device, std::span(vertexData), false);
			cube.material = rs->simpleMat;
		}

		{
			const auto& meshData = *resourceManager->LoadModel("Data/Models/BoxTextured/BoxTextured.gltf");
			const auto& submesh = meshData[0];

			const u64 vertexCount = submesh.v.size();
			const u64 indexCount = submesh.i.size();

			auto vertexFormatData = std::vector<VertexPosUV>(vertexCount);
			{
				for (int i = 0; i < vertexCount; i++) {
					const auto& v = submesh.v[i];
					vertexFormatData[i] = VertexPosUV{
						.Position = XMFLOAT3(v.position.x, v.position.y, v.position.z),
						.Normal = XMFLOAT3(v.normal.x, v.normal.y, v.normal.z),
						.UV = XMFLOAT2(v.uv[0].x, v.uv[0].y)
					};
				}
			}

			auto& box = rs->debugBoxTextured;
			box = DescribedMesh{
				.transform = {
					.position = { 1.0f, 1.0f, 1.0f },
					.scale = {4.1f, 4.1f, 4.1f}
				},
				.mesh = submesh, // TODO: is this useless?
				.gpuData = GPUMeshData{
					.vertexCount = vertexCount,
					.indexCount = indexCount,
					.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
				},
				.material = rs->textureMat
			};

			auto x = submesh.i;
			box.gpuData.indexBuffer.Create(device, std::span(x));
			box.gpuData.vertexBuffer.Create<VertexPosUV>(device, std::span(vertexFormatData), false);
		}

		return true;
	}

	auto SetDefaultPass(const DXLayer::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void {
		Assert(cmd.queue != nullptr);
		cmd.queue->OMSetRenderTargets(1, renderTargetView, &depthStencilView);
	}
}