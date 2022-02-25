#pragma once

#include "../platform.h"
#include "../Renderer/renderer.h"
#include "../Math.h"
#include "../ObjLoader.h"
#include "../ResourceManager.h"
#include <vector>

namespace Nickel::App {
	static struct LineVertexData {
		XMFLOAT3 position;
		XMFLOAT3 previous;
		XMFLOAT3 next;
		XMFLOAT3 direction;
	};

	auto GenerateLine(RendererState* rs, std::vector<Vec3> vertexData, DescribedMesh& describedMesh) -> void;
	auto GenerateLinePoints(Vec3 from, Vec3 to, u32 pointCount) -> std::vector<Vec3>;
	auto CreateLineIndices(u32 length) -> std::vector<u32>;
	auto DuplicateLineVertices(std::span<Vec3> vertices, bool mirror = false) -> std::vector<Vec3>;
	auto DuplicateLineVertices(std::span<i32> vertices, bool mirror = false) -> std::vector<i32>;
	//auto GenerateLineInDir(Vec3 startPos, Vec3 pointOffset, Vec3 dir, u32 pointCount) -> std::vector<Vec3>;
}