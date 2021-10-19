#pragma once

#include "DirectXIncludes.h"
#include <variant>

namespace Nickel::Renderer::DXLayer {
	enum class ShaderType {
		Vertex,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute
	};

	/*
	auto TypeOfShader = Overload {
		[](ID3D11VertexShader*)   { return ID3D11VertexShader* },
		[](ID3D11HullShader*)     { return decltype(int); },
		[](ID3D11DomainShader*)   { return decltype(int); },
		[](ID3D11GeometryShader*) { return decltype(int); },
		[](ID3D11PixelShader*)    { return decltype(int); },
		[](ID3D11ComputeShader*)  { return decltype(int); },
	};
	*/

	// using ShaderVariant = std::variant<ID3D11VertexShader*, ID3D11HullShader*, ID3D11DomainShader*, ID3D11GeometryShader*, ID3D11PixelShader*, ID3D11ComputeShader*>;

	//struct Shader {
		//ShaderType type;
		//ShaderVariant shader;
	//};

	// HasProperty
	// GetPropertyLocation
	// SetProperty
	class ShaderProgram {
		public:
		ShaderProgram() = default;
		~ShaderProgram() = default;

		auto Create(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> vertexShaderBytecode, std::span<const u8> pixelShaderBytecode) -> void;
		auto Bind(ID3D11DeviceContext1* ctx) -> void;
		auto Unbind(ID3D11DeviceContext1* ctx) -> void;

		ID3D11InputLayout* inputLayout;
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;

		/*
		Shader shaders[6] = {
			Shader{.type = ShaderType::Vertex},
			Shader{.type = ShaderType::Hull},
			Shader{.type = ShaderType::Domain},
			Shader{.type = ShaderType::Geometry},
			Shader{.type = ShaderType::Pixel},
			Shader{.type = ShaderType::Compute}
		};
		*/

		private:

		template<typename ShaderT>
		static auto CreateShaderFromBytecode(ID3D11Device1* device, ShaderT& shader, std::span<const u8> shaderBytecode) -> void {
			using T = std::decay_t<decltype(shader)>;
			if (std::is_same_v<T, ID3D11VertexShader*>) {
				ASSERT_ERROR_RESULT(device->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11VertexShader**>(&shader)));
			} else if  (std::is_same_v<T, ID3D11HullShader*>) {
				ASSERT_ERROR_RESULT(device->CreateHullShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11HullShader**>(&shader)));
			} else if  (std::is_same_v<T, ID3D11DomainShader*>) {
				ASSERT_ERROR_RESULT(device->CreateDomainShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11DomainShader**>(&shader)));
			} else if  (std::is_same_v<T, ID3D11GeometryShader*>) {
				ASSERT_ERROR_RESULT(device->CreateGeometryShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&shader)));
			} else if  (std::is_same_v<T, ID3D11PixelShader*>) {
				ASSERT_ERROR_RESULT(device->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11PixelShader**>(&shader)));
			} else if  (std::is_same_v<T, ID3D11ComputeShader*>) {
				ASSERT_ERROR_RESULT(device->CreateComputeShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11ComputeShader**>(&shader)));
			} else {
				Logger::Error("[ShaderProgram]: Unhandled shader stage");
				Assert(false);
			}
		}
	};
}