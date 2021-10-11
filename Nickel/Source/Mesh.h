#pragma once
#include <d3d11.h>
#include <vector>
#include "platform.h"

namespace Nickel {
	struct MeshData {
		MeshData() {
			v = std::vector<f64>();
			i = std::vector<u32>();
			n = std::vector<f64>();
			uv = std::vector<f32>();
		}

		std::vector<f64> v;
		std::vector<u32> i;
		std::vector<f64> n;
		std::vector<f32> uv;
	};

	class Model {
		D3D11_PRIMITIVE_TOPOLOGY topologyType;
	};
}