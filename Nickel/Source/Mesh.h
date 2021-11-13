#pragma once
#include <d3d11.h>
#include <vector>
#include "platform.h"
#include "Math.h"

namespace Nickel {
	struct LoadedVertex {
		Vec3 position;
		Vec3 normal;
		Vec3 tangent;
		Vec3 bitangent;
		Vec2 uv[8];
		Vec4 colors[8];
	};

	struct MeshData {
		std::vector<LoadedVertex> v = std::vector<LoadedVertex>();
		std::vector<u32> i = std::vector<u32>();
	};

	class Model {
		D3D11_PRIMITIVE_TOPOLOGY topologyType;
	};
}