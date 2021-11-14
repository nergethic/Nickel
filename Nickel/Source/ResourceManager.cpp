#define STB_IMAGE_IMPLEMENTATION
#include "ResourceManager.h"

namespace Nickel::Renderer {
	ResourceManager* ResourceManager::resourceManager = nullptr;

	auto ResourceManager::GetInstance() -> ResourceManager* {
		if (resourceManager == nullptr)
			resourceManager = new ResourceManager();

		return resourceManager;
	}

	auto ResourceManager::Init(ID3D11Device1* _device) -> void {
		device = _device;

		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));

		D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR; //D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		desc.MaxAnisotropy = 0;
		desc.MaxLOD = FLT_MAX;
		desc.MinLOD = FLT_MIN;
		desc.MipLODBias = 0;

		defaultSamplerState = DXLayer::CreateSamplerState(device, desc);
	}

	auto ResourceManager::LoadTexture(std::wstring path) -> DXLayer::TextureDX11 {
		DXLayer::TextureDX11 newTex{ .samplerState = defaultSamplerState };
		ASSERT_ERROR_RESULT(DirectX::CreateWICTextureFromFile(device, path.c_str(), &newTex.resource, &newTex.srv));
		Assert(newTex.srv != nullptr);

		return newTex;
	}

	auto ResourceManager::LoadImageData(std::string path) -> LoadedImageData {
		i32 width, height, channels;
		u8* img = stbi_load((path.c_str()), &width, &height, &channels, STBI_rgb_alpha);
		Assert(img != nullptr);

		return {
			img, static_cast<u32>(width), static_cast<u32>(height)
		};
	}

	auto ResourceManager::ProcessMesh(const aiMesh& mesh, const aiScene& scene) -> MeshData {
		auto vertices = std::vector<LoadedVertex>(mesh.mNumVertices);

		Assert(mesh.HasPositions());
		for (u64 i = 0; i < mesh.mNumVertices; i++) {
			const auto& vert = mesh.mVertices[i];
			vertices[i].position = { vert.x, vert.y, vert.z };
		}

		if (mesh.HasNormals()) {
			for (u64 i = 0; i < mesh.mNumVertices; i++) {
				const auto& normal = mesh.mNormals[i];
				vertices[i].normal = Vec3{ normal.x, normal.y, normal.z };
			}

			if (mesh.HasTangentsAndBitangents()) {
				for (u64 i = 0; i < mesh.mNumVertices; i++) {
					const auto& tangent = mesh.mTangents[i];
					const auto& bitangent = mesh.mBitangents[i];
					vertices[i].tangent = Vec3{ tangent.x, tangent.y, tangent.z };
					vertices[i].bitangent = Vec3{ bitangent.x, bitangent.y, bitangent.z };
				}
			}
		}

		const u32 uvChannelsCount = mesh.GetNumUVChannels();
		//for (u32 channelIdx = 0; channelIdx < uvChannelsCount; channelIdx++) {
			//if (!mesh.HasTextureCoords(channelIdx))
				//continue;

		for (u64 i = 0; i < mesh.mNumVertices; i++) {
			if (mesh.mTextureCoords[0] == nullptr) {
				vertices[i].uv[0] = Vec2{ 0.0f, 0.0f };
				continue;
			}
			const auto& uv = mesh.mTextureCoords[0][i];
			vertices[i].uv[0] = Vec2{ uv.x, uv.y };
		}
		//}

		const u32 colorChannelsCount = mesh.GetNumColorChannels();
		for (u32 i = 0; i < uvChannelsCount; i++) {
			if (!mesh.HasVertexColors(i))
				continue;

			for (u64 j = 0; j < mesh.mNumVertices; j++) {
				const auto& color = mesh.mColors[i][j];
				vertices[i].colors[j] = Vec4{ color.r, color.g, color.b, color.a };
			}
		}

		auto indices = std::vector<u32>(mesh.mNumFaces * 3);
		for (u32 i = 0; i < mesh.mNumFaces; i++) {
			aiFace face = mesh.mFaces[i];
			Assert(face.mNumIndices == 3); // NOTE: mesh must be triangulated
			for (u32 j = 0; j < 3; j++)
				indices[(i * 3) + j] = face.mIndices[j];
		}

		// process materials
		//aiMaterial* mat = scene->mMaterials[mesh.mMaterialIndex];
		//mat->GetTexture(aiTextureType::)
		//std::vector<Texture> textures;

		return MeshData{ .v = vertices, .i = indices }; // textures
	}

	auto ResourceManager::ProcessNode(aiNode* node, const aiScene& scene, std::vector<MeshData>& submeshes) -> void {
		if (submeshes.size() == 5) {
			int x = 4;
		}
		for (u32 i = 0; i < node->mNumMeshes; i++) {
			const aiMesh* submesh = scene.mMeshes[node->mMeshes[i]];
			submeshes.push_back(ProcessMesh(*submesh, scene));
		}

		for (u32 i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene, submeshes);
		}
	}

	auto ResourceManager::LoadModel(std::string path) -> std::vector<MeshData>* {
		Assimp::Importer importer;
		//const u32 flags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices |
			//aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;
		const u32 flags = aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace;
		const aiScene* scene = importer.ReadFile(path, flags); // aiProcess_FlipUVs aiProcess_JoinIdenticalVertices
		if (scene == nullptr) {
			Logger::Error(importer.GetErrorString());
			return nullptr;
		}

		std::vector<MeshData>* submeshes = new std::vector<MeshData>();
		ProcessNode(scene->mRootNode, *scene, *submeshes);

		return submeshes;
	}

	auto ResourceManager::GetDefaultSamplerState() -> ID3D11SamplerState* {
		return defaultSamplerState;
	}
}