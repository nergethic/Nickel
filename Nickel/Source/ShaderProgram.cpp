#include "ShaderProgram.h"

namespace Nickel::Renderer::DXLayer {
	auto CreateInputLayout(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> shaderBytecodeWithInputSignature) -> ID3D11InputLayout*;

	auto ShaderProgram::Create(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> vertexShaderBytecode, std::span<const u8> pixelShaderBytecode) -> void {
		Assert(vertexShaderBytecode.data() != nullptr);
		Assert(pixelShaderBytecode.data() != nullptr);

		inputLayout = CreateInputLayout(device, vertexLayoutDesc, vertexShaderBytecode);
		CreateShaderFromBytecode(device, vertexShader, vertexShaderBytecode);
		CreateShaderFromBytecode(device, pixelShader, pixelShaderBytecode);
	}

	auto ShaderProgram::Bind(ID3D11DeviceContext1* ctx) -> void {
		
		auto bindShader = [&](auto& arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, ID3D11VertexShader*>) {
				ctx->VSSetShader(arg, nullptr, 0);
			} else if constexpr (std::is_same_v<T, ID3D11HullShader*>) {
				ctx->PSSetShader(arg, nullptr, 0);
			} else if constexpr (std::is_same_v<T, ID3D11DomainShader*>) {
				ctx->DSSetShader(arg, nullptr, 0);
			} else if constexpr (std::is_same_v<T, ID3D11GeometryShader*>) {
				ctx->GSSetShader(arg, nullptr, 0);
			} else if constexpr (std::is_same_v<T, ID3D11PixelShader*>) {
				ctx->PSSetShader(arg, nullptr, 0);
			} else if constexpr (std::is_same_v<T, ID3D11ComputeShader*>) {
				ctx->CSSetShader(arg, nullptr, 0);
			} else {
				Assert(false);
				Logger::Error("[ShaderProgram]: Unhandled shader stage");
			}
		};

		bindShader(vertexShader);
		bindShader(pixelShader);

		// TODO:
		//for (auto& s : shaders)
			//bindShader(s);
	}

	auto ShaderProgram::Unbind(ID3D11DeviceContext1* ctx) -> void {
		ctx->VSSetShader(nullptr, nullptr, 0);
		ctx->PSSetShader(nullptr, nullptr, 0);
	}
}