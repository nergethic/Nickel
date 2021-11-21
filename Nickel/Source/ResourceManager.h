#pragma once
#include "Renderer/DX11Layer.h"
#include "Mesh.h"
#include "stb/stb_image.h"

#pragma comment(lib, "assimp-vc143-mtd.lib")
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//#include "assimp/material.h"

namespace Nickel::Renderer {
	//struct Texture {
		//unsigned int id;
		//std::string type;
		//std::string path;  // we store the path of the texture to compare with other textures
	//};

	class ResourceManager {
	private:
		static ResourceManager* resourceManager;
		ResourceManager() = default;

	public:
		ResourceManager(ResourceManager& other) = delete;
		auto operator=(const ResourceManager&) -> void = delete;

		static auto GetInstance() -> ResourceManager*;
		auto Init(ID3D11Device1* _device) -> void;
		
		// NOTE: supports BMP, JPEG, PNG, TIFF, GIF
		auto LoadTexture(std::wstring path)->DXLayer11::TextureDX11;
		auto LoadImageData(std::string path)->LoadedImageData;
		auto LoadHDRImageData(std::string path)->LoadedImageData;
		auto ProcessMesh(const aiMesh& mesh, const aiScene& scene)->MeshData;
		auto ProcessNode(aiNode* node, const aiScene& scene, std::vector<MeshData>& submeshes) -> void;
		auto LoadModel(std::string path)->std::vector<MeshData>*;
		auto GetDefaultSamplerState()->ID3D11SamplerState*;

		/*
		inline std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
			std::vector<Texture> textures;
			for (u32 i = 0; i < mat->GetTextureCount(type); i++) {
				aiString str;
				mat->GetTexture(type, i, &str);
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
			}
			return textures;
		}
		*/

	private:
		ID3D11Device1* device;
		std::vector<DXLayer11::TextureDX11> loadedTextures = std::vector<DXLayer11::TextureDX11>();
	};
}