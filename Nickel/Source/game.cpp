#include "game.h"
#include "Renderer/renderer.h"
#include "Renderer/Direct3D11/D3D11Core.h"

#include "LineGenerator.h"

namespace Nickel {
	using namespace Renderer;

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

	static Nickel::Renderer::Surface surface;
	auto NewInitialize(GameMemory* memory) -> void {
		auto resourceManager = ResourceManager::GetInstance();
		resourceManager->Init();

		auto window = Platform::Window{0};
		surface = Renderer::CreateSurface(window);
	}

	auto Initialize(GameMemory* memory, RendererState* rs) -> void {
		Assert(memory != nullptr);
		Assert(rs != nullptr);
		Assert(rs->device);
		Assert(rs->cmdQueue.queue);

		ID3D11Device1* device = rs->device.Get();
		auto resourceManager = ResourceManager::GetInstance();
		resourceManager->Init();

		rs->mainCamera = std::make_unique<Camera>(45.0f, 1.5f, 0.1f, 100.0f);

		background.Create();

		rs->defaultDepthStencilBuffer = DX11Layer::CreateDepthStencilTexture(device, rs->backbufferWidth, rs->backbufferHeight);
		Assert(rs->defaultDepthStencilBuffer != nullptr);
		rs->defaultDepthStencilView = DX11Layer::CreateDepthStencilView(device, rs->defaultDepthStencilBuffer);

		// Create the constant buffers for the variables defined in the vertex shader.
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Appliation] = DX11Layer::CreateConstantBuffer(device, sizeof(PerApplicationData));
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Object] = DX11Layer::CreateConstantBuffer(device, sizeof(PerObjectBufferData));
		rs->g_d3dConstantBuffers[(u32)ConstantBufferType::CB_Frame] = DX11Layer::CreateConstantBuffer(device, sizeof(PerFrameBufferData));

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

		//rs->albedoTexture = resourceManager->LoadTexture(L"Data/Models/HornetHelmet/textures/03___Default_baseColor.jpg");
		//rs->normalTexture = resourceManager->LoadTexture(L"Data/Models/HornetHelmet/textures/03___Default_normal.jpg");
		//rs->aoTexture = resourceManager->LoadTexture(L"Data/Models/HornetHelmet/Default_AO.jpg");
		//rs->metalRoughnessTexture = resourceManager->LoadTexture(L"Data/Models/HornetHelmet/textures/03___Default_metallicRoughness.png");
		//rs->emissiveTexture = resourceManager->LoadTexture(L"Data/Models/HornetHelmet/textures/03___Default_emissive.jpg");

		rs->debugBoxTexture = resourceManager->LoadTexture(L"Data/Models/BoxTextured/CesiumLogoFlat.png");

		rs->matCapTexture = resourceManager->LoadTexture(L"Data/Textures/matcap.jpg");

		const wchar_t* radianceFacePaths[6] = {
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posx.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negx.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posy.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negy.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posz.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negz.hdr"
		};
		rs->radianceTexture = DX11Layer::CreateCubeMap(device, radianceFacePaths);
		rs->brdfLUT = resourceManager->LoadTexture(L"Data/Textures/brdfLUT.jpg");

		auto defaultDepthStencilState = DX11Layer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
		auto defaultRasterizerState = DX11Layer::CreateDefaultRasterizerState(device);

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
		
		
		// simpleMat.constantBuffer = DX11Layer::CreateConstantBuffer(device, )
		{
			auto& textureMat = rs->textureMat;
			textureMat = Material{
				.program = &rs->textureProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				}
			};
			textureMat.textures = std::vector<DX11Layer::TextureDX11>(1);
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
					.buffer = DX11Layer::CreateConstantBuffer(device, sizeof(PbrPixelBufferData)),
					.index = 3
				}
			};
			pbrMat.textures = std::vector<DX11Layer::TextureDX11>(8);
			pbrMat.textures[0] = rs->albedoTexture;
			pbrMat.textures[1] = rs->normalTexture;
			pbrMat.textures[2] = rs->metalRoughnessTexture;
			pbrMat.textures[3] = rs->aoTexture;
			pbrMat.textures[4] = rs->emissiveTexture;
			pbrMat.textures[5] = background.texture;
			pbrMat.textures[6] = rs->radianceTexture;
			pbrMat.textures[7] = rs->brdfLUT;

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

	auto NewUpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void {
		timer += 0.01f;
		if (timer > 1000.0f)
			timer -= 1000.0f;

		static float angle = -90.0f;
		float radius = 2.1f;
		angle += 1.2f;
		XMFLOAT3 lightPos = XMFLOAT3(radius * cos(XMConvertToRadians(angle)), 0.0f, radius * sin(XMConvertToRadians(angle)));

		f32 dtMouseX = input->normalizedMouseX - previousMouseX;
		f32 dtMouseY = input->normalizedMouseY - previousMouseY;

		// Update camera
		auto& camera = *Renderer::DX11Layer::Core::GetMainCamera();
		camera.lookAtPosition.x += dtMouseX * 50.0f;
		camera.lookAtPosition.y += dtMouseY * 50.0f;
		camera.RecalculateMatrices();

		// Prepare and send frame data to the GPU
		PerFrameBufferData frameData;
		frameData.viewMatrix = XMMatrixTranspose(camera.GetViewMatrix());
		frameData.cameraPosition = XMFLOAT3(camera.position.x, camera.position.y, camera.position.z);
		frameData.lightPosition = lightPos;

		auto cmd = Renderer::DX11Layer::Core::GetCmd();
		cmd->UpdateSubresource1(Renderer::DX11Layer::Core::GetPerFrameUniform(), 0, nullptr, &frameData, 0, 0, 0);

		// Render
		if (surface.IsValid())
			surface.Render();

		previousMouseX = input->normalizedMouseX;
		previousMouseY = input->normalizedMouseY;
	}

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
			.lightColors = {XMFLOAT4(50.0f * 0.0392f, 50.0f * 0.0392f, 50.0f * 0.0512f, 0), XMFLOAT4(0.0392f, 0.0392f, 0.0512f, 0), XMFLOAT4(0.0392f, 0.0392f, 0.0512f, 0), XMFLOAT4(200.0f, 200.0f, 200.0f, 0)},
			.albedoFactor = XMFLOAT4(0.2f, 0.05f, 0.75f, 0.0f),
			.metallic = 0.6f,
			.roughness = 0.4f,
			.ao = 1.0f
		};
		rs->pbrMat.pixelConstantBuffer.Update(rs->cmdQueue.queue.Get(), bufferData);

		// RENDER ---------------------------
		/*
		for (u32 i{ 0 }; i < _countof(_surfaces); ++i) {
			if (_surfaces[i].surface.is_valid()) {
				_surfaces[i].surface.render();
			}
		}
		*/

		Assert(rs->defaultRenderTargetView != nullptr);
		Assert(rs->defaultDepthStencilView != nullptr);
		DX11Layer::SetRenderTarget(queue, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
		queue.RSSetViewports(1, &rs->g_Viewport);

		DX11Layer::ClearFlag clearFlag = DX11Layer::ClearFlag::CLEAR_COLOR | DX11Layer::ClearFlag::CLEAR_DEPTH;
		DX11Layer::Clear(rs->cmdQueue, static_cast<u32>(clearFlag), rs->defaultRenderTargetView, rs->defaultDepthStencilView, clearColor, 1.0f, 0);

		//auto& camera = *rs->mainCamera.get();

		/*
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Hello imgui    ");
		ImGui::Text("Lorem Ipsum     ");
		ImGui::End();
		*/
		
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

		/*
		for (auto& line : rs->lines)
			if (line.material.program != nullptr)
				DrawModel(camera, cmd, line);

		for (int y = -2; y <= 2; y++) {
			for (int x = -2; x <= 2; x++) {
				for (int i = 0; i < rs->bunny.size(); i++) {
					const auto t = rs->bunny[i].transform;
					rs->bunny[i].transform.rotation.y += 0.0004f;
					DrawModel(camera, cmd, rs->bunny[i], {x * 3.0f, y * 3.0f, -4.0f});
				}
			}
		}

		DrawModel(camera, cmd, rs->debugCube);
		*/
		// DrawModel(*rs, cmd, rs->debugBoxTextured);

		/*
		f32 dtMouseX = input->normalizedMouseX - previousMouseX;
		f32 dtMouseY = input->normalizedMouseY - previousMouseY;
		camera.lookAtPosition.x += dtMouseX * 50.0f;
		camera.lookAtPosition.y += dtMouseY * 50.0f;
		camera.RecalculateMatrices();

		DrawModel(camera, cmd, background.skyboxMesh);
		*/
		
		/*
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		*/
		//rs->swapChain->Present(1, 0);

		previousMouseX = input->normalizedMouseX;
		previousMouseY = input->normalizedMouseY;
	}

	auto LoadContent(RendererState* rs) -> bool {
		Assert(rs != nullptr);
		Assert(rs->device != nullptr);
		Assert(rs->cmdQueue.queue != nullptr);

		auto device = rs->device.Get();
		auto resourceManager = ResourceManager::GetInstance();

		{ // convert vertex data to be LineVertexData compatible
			auto defaultDepthStencilState = DX11Layer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
			auto rasterizerDesc = DX11Layer::GetDefaultRasterizerDescription();
			rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
			//rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
			auto defaultRasterizerState = DX11Layer::CreateRasterizerState(device, rasterizerDesc);
			auto& lineMat = rs->lineMat;
			lineMat = Material{
				.program = &rs->lineProgram,
				.pipelineState = PipelineState{
					.rasterizerState = defaultRasterizerState,
					.depthStencilState = defaultDepthStencilState
				},
				.vertexConstantBuffer = {
					.buffer = DX11Layer::CreateConstantBuffer(device, sizeof(LineBufferData)),
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
				App::GenerateLine(rs, App::GenerateLinePoints(Vec3{ xOffset, 0.0, -10.0 }, Vec3{ xOffset, 0.0, 10.0 }, linePointsCount), rs->lines[lineIdx]);
			}
				
			for (i32 y = -5; y <= 5; y++, lineIdx++) {
				const auto yOffset = static_cast<f32>(y) * 2.0f;
				App::GenerateLine(rs, App::GenerateLinePoints(Vec3{ -10.0, 0.0, yOffset }, Vec3{ 10.0, 0.0, yOffset }, linePointsCount), rs->lines[lineIdx]);
			}
		}

		{ // Bunny
			//LoadMeshAndSetup("Data/Models/bny.obj");

			//MeshData meshData;
			// LoadBunnyMesh(meshData);
			//const auto& meshData = *resourceManager->LoadModel("Data/Models/backpack/backpack.obj");
			const auto& meshData = *resourceManager->LoadModel("Data/Models/DamagedHelmet/DamagedHelmet.gltf");
			//const auto& meshData = *resourceManager->LoadModel("Data/Models/HornetHelmet/scene.gltf");
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

	auto SetDefaultPass(const DX11Layer::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void {
		Assert(cmd.queue != nullptr);
		cmd.queue->OMSetRenderTargets(1, renderTargetView, &depthStencilView);
	}
}