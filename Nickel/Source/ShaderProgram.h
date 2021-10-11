#pragma once
#include <d3d11.h>

// bind uniform float
// bind uniform vector etc
// get location

namespace Nickel {
	class ShaderProgram {
		ID3D11VertexShader* vertexShader = nullptr;
		ID3D11PixelShader* pixelShader = nullptr;

		public:
			ShaderProgram();
			~ShaderProgram();
	};
}