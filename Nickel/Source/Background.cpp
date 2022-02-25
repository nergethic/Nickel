#include "Background.h"
#include "Renderer/Direct3D11/D3D11Core.h"

namespace Nickel {
	auto Background::Create() -> void {
		auto device = Renderer::DX11Layer::Core::GetDevice();
		shaderProgram.Create(device, std::span{ g_BackgroundVertexShader }, std::span{ g_BackgroundPixelShader });
		material = CreateMaterial(device);
		texture = CreateCubemapTexture(device, texturePath);
		material.textures[0] = texture;

		skyboxMesh = CreateSkyboxMesh(device);
		skyboxMesh.material = material;
	};

	auto Background::Bind() -> void {
		auto cmd = Renderer::DX11Layer::Core::GetCmd();
		irradianceMaterial.program->Bind(cmd);
		//irradianceMaterial.pixelConstantBuffer.Set
		/*
		irradianceShader.use();
		irradianceShader.setInt("environmentMap", 0);
		irradianceShader.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			irradianceShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*/
	}

	auto Background::CreateCubemapTexture(ID3D11Device1* device, const std::string& path) -> DX11Layer::TextureDX11 {
		const auto rm = ResourceManager::GetInstance();
		/*
		LoadedImageData imgs[6] {
			rm->LoadHDRImageData(path + "_posx.hdr"),
			rm->LoadHDRImageData(path + "_negx.hdr"),
			rm->LoadHDRImageData(path + "_posy.hdr"),
			rm->LoadHDRImageData(path + "_negy.hdr"),
			rm->LoadHDRImageData(path + "_posz.hdr"),
			rm->LoadHDRImageData(path + "_negz.hdr")
		};
		*/

		//L"Data/Textures/skybox/radianceCubemap/output_iem_posx.hdr",
		const wchar_t* files[6] = {
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posx.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negx.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posy.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negy.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_posz.hdr",
			L"Data/Textures/skybox/radianceCubemap/output_pmrem_negz.hdr"
		};
			
		auto result = DX11Layer::CreateCubeMap(device, files);
		//for (u32 i = 0; i < ArrayCount(imgs); i++)
			//stbi_image_free(imgs[i].data);

		return result;
	}

	auto Background::CreateMaterial(ID3D11Device1* device) -> Material {
		auto rasterizerDesc = DX11Layer::GetDefaultRasterizerDescription();
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;

		return Material{
			.program = &shaderProgram,
			.pipelineState = PipelineState{
				.rasterizerState = DX11Layer::CreateRasterizerState(device, rasterizerDesc),
				.depthStencilState = DX11Layer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, false)
			},
			.textures = std::vector<DX11Layer::TextureDX11>(1)
		};
	}

	auto Background::CreateSkyboxMesh(ID3D11Device1* device) -> DescribedMesh {
		const float side = 0.5f;
		auto vertexData = std::vector<VertexPos>(8);
		vertexData[0] = VertexPos{ .Position = {-side,-side,-side} };
		vertexData[1] = VertexPos{ .Position = {side,-side,-side} };
		vertexData[2] = VertexPos{ .Position = {-side,side,-side} };
		vertexData[3] = VertexPos{ .Position = {side,side,-side} };
		vertexData[4] = VertexPos{ .Position = {-side,-side,side} };
		vertexData[5] = VertexPos{ .Position = {side,-side,side} };
		vertexData[6] = VertexPos{ .Position = {-side,side,side} };
		vertexData[7] = VertexPos{ .Position = {side,side,side} };

		auto indices = std::vector<u32>{
			0,2,1, 2,3,1,
			1,3,5, 3,7,5,
			2,6,3, 3,6,7,
			4,5,7, 4,7,6,
			0,4,2, 2,4,6,
			0,1,4, 1,5,4
		};

		/* // NOTE: This was causing internal compiler error...
		auto skybox = DescribedMesh{
			.transform = {
				.position = { 0.0f, 0.0f, 0.0f },
				.scale = {1.0f, 1.0f, 1.0f},
				.rotation = {0.0f, 0.0f, 0.0f}
			},
			.gpuData = GPUMeshData{
				.vertexCount = vertexCount,
				.indexCount = indexCount
			}
		};
		*/

		DescribedMesh skybox{
			.transform = {
				.position = { 0.0f, 0.0f, 0.0f },
				.scale = {1.0f, 1.0f, 1.0f},
				.rotation = {0.0f, 0.0f, 0.0f}
			}
		};
			
		skybox.gpuData.indexBuffer.Create(device, std::span{indices});
		skybox.gpuData.indexCount = static_cast<u32>(indices.size());
		skybox.gpuData.vertexBuffer.Create<VertexPos>(device, std::span(vertexData), false);
		skybox.gpuData.vertexCount = static_cast<u32>(vertexData.size());

		return skybox;
	}
}