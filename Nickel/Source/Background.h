#pragma once

#include "Renderer/renderer.h"
#include "Math.h"
#include "ResourceManager.h"

namespace Nickel {
	struct VertexPos {
		XMFLOAT3 Position;
	};

	class Background {
		const std::string texturePath = "Data/Textures/skybox/radianceCubemap/output_pmrem";
		// const std::string texturePath = "Data/Textures/skybox/irradianceCubemap/output_pmrem"; // /galaxy2048.jpg

		DX11Layer::ShaderProgram shaderProgram;
		Material material;

		DX11Layer::ShaderProgram irradianceshaderProgram;
		Material irradianceMaterial;

	public:
		DX11Layer::TextureDX11 texture;
		DescribedMesh skyboxMesh;

		auto Create() -> void;
		auto Bind() -> void;

	private:
		auto CreateCubemapTexture(ID3D11Device1* device, const std::string& path) -> DX11Layer::TextureDX11;
		auto CreateMaterial(ID3D11Device1* device) -> Material;
		auto CreateSkyboxMesh(ID3D11Device1* device) -> DescribedMesh;
	};
}