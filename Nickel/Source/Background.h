#pragma once
#include "renderer.h"
#include "Math.h"
#include "ResourceManager.h"

namespace Nickel {
	struct VertexPos {
		XMFLOAT3 Position;
	};

	class Background {
		const std::string texturePath = "Data/Textures/skybox/irradianceCubemap/output_pmrem"; // /galaxy2048.jpg

		DXLayer::ShaderProgram shaderProgram;
		Material material;

		DXLayer::ShaderProgram irradianceshaderProgram;
		Material irradianceMaterial;

	public:
		DXLayer::TextureDX11 texture;
		DescribedMesh skyboxMesh;

		inline auto Create(ID3D11Device1* device) {
			shaderProgram.Create(device, std::span{ g_BackgroundVertexShader }, std::span{ g_BackgroundPixelShader });
			material = CreateMaterial(device);
			texture = CreateCubemapTexture(device, texturePath);
			material.textures[0] = texture;

			skyboxMesh = CreateSkyboxMesh(device);
			skyboxMesh.material = material;
		};

		inline auto Bind(ID3D11DeviceContext1* cmd) -> void {
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

	private:
		inline auto CreateCubemapTexture(ID3D11Device1* device, const std::string& path) -> DXLayer::TextureDX11 {
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

			const wchar_t* files[6] = {
				L"Data/Textures/skybox/irradianceCubemap/output_iem_posx.hdr",
				L"Data/Textures/skybox/irradianceCubemap/output_iem_negx.hdr",
				L"Data/Textures/skybox/irradianceCubemap/output_iem_posy.hdr",
				L"Data/Textures/skybox/irradianceCubemap/output_iem_negy.hdr",
				L"Data/Textures/skybox/irradianceCubemap/output_iem_posz.hdr",
				L"Data/Textures/skybox/irradianceCubemap/output_iem_negz.hdr"
			};
			
			auto result = DXLayer::CreateCubeMap(device, files);
			//for (u32 i = 0; i < ArrayCount(imgs); i++)
				//stbi_image_free(imgs[i].data);

			return result;
		}

		inline auto CreateMaterial(ID3D11Device1* device) -> Material {
			auto rasterizerDesc = DXLayer::GetDefaultRasterizerDescription();
			rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;

			return Material{
				.program = &shaderProgram,
				.pipelineState = PipelineState{
					.rasterizerState = DXLayer::CreateRasterizerState(device, rasterizerDesc),
					.depthStencilState = DXLayer::CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, false)
				},
				.textures = std::vector<DXLayer::TextureDX11>(1)
			};
		}

		inline auto CreateSkyboxMesh(ID3D11Device1* device) -> DescribedMesh {
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

			const auto vertexCount = static_cast<u32>(vertexData.size());
			const auto indexCount = static_cast<u32>(indices.size());

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
			
			skybox.gpuData.indexBuffer.Create(device, std::span{ indices });
			skybox.gpuData.vertexBuffer.Create<VertexPos>(device, std::span(vertexData), false);

			return skybox;
		}
	};
}