#include "ShaderProgram.h"

namespace Nickel::Renderer::DXLayer {
	auto ShaderProgram::Create(ID3D11Device1* device, const void* vertexShaderBytecode, const void* pixelShaderBytecode) -> void {
		Assert(vertexShaderBytecode != nullptr);
		Assert(pixelShaderBytecode != nullptr);
		vertexShader = CreateShaderFromBytecode<ID3D11VertexShader>(device, ShaderStage::Vertex, vertexShaderBytecode);
		pixelShader = CreateShaderFromBytecode<ID3D11PixelShader>(device, ShaderStage::Pixel, pixelShaderBytecode);
	}

	auto ShaderProgram::Bind(ID3D11DeviceContext1* ctx) -> void {
		ctx->VSSetShader(vertexShader, nullptr, 0);
		ctx->PSSetShader(pixelShader, nullptr, 0);
	}

	auto ShaderProgram::Unbind(ID3D11DeviceContext1* ctx) -> void {
		ctx->VSSetShader(nullptr, nullptr, 0);
		ctx->PSSetShader(nullptr, nullptr, 0);
	}
}