#include "LineGenerator.h"

namespace Nickel::App {
	namespace internal {
		auto Clamp(u32 val, u32 min, u32 max) {
			if (val < min)
				val = min;
			else if (val > max)
				val = max;

			return val;
		}
	}

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
			u32 idx = internal::Clamp(i - 1, 0, prevs.size() - 1);
			prevs[i] = vertexData[idx];
		}
		auto previousVertex = DuplicateLineVertices(std::span{ prevs });

		auto nexts = std::vector<Vec3>(vertexData.size());
		for (u32 i = 0; i < vertexData.size(); i++) {
			u32 idx = internal::Clamp(i + 1, 0, nexts.size() - 1);
			nexts[i] = vertexData[idx];
		}
		auto nextVertex = DuplicateLineVertices(std::span{ nexts });

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

		describedMesh.transform.scale = { 1.0f, 1.0f, 1.0f };
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

	auto CreateLineIndices(u32 length) -> std::vector<u32> {
		auto indices = std::vector<u32>(length * 6);

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

	auto DuplicateLineVertices(std::span<Vec3> vertices, bool mirror) -> std::vector<Vec3> {
		Assert(vertices.data() != nullptr && vertices.size() >= 0);
		auto result = std::vector<Vec3>();
		for (const auto& v : vertices) {
			const auto outV = mirror ? Vec3(-v.x, -v.y, -v.z) : v;
			result.push_back(outV);
			result.push_back(v);
		}
		return result;
	}

	auto DuplicateLineVertices(std::span<i32> vertices, bool mirror) -> std::vector<i32> {
		Assert(vertices.data() != nullptr && vertices.size() >= 0);
		auto result = std::vector<i32>();
		for (const auto& v : vertices) {
			const auto outV = mirror ? -v : v;
			result.push_back(outV);
			result.push_back(v);
		}
		return result;
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
}