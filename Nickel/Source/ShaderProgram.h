#pragma once

#include "DirectXIncludes.h"
// bind uniform float
// bind uniform vector etc
// get location

namespace Nickel::Renderer::DXLayer {
	enum class ShaderStage {
		Vertex,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute
	};

	class ShaderProgram {
		public:
		ShaderProgram() = default;
		~ShaderProgram() = default;

		auto Create(ID3D11Device1* device, const void* vertexShaderBytecode, const void* pixelShaderBytecode) -> void;
		auto Bind(ID3D11DeviceContext1* ctx) -> void;
		auto Unbind(ID3D11DeviceContext1* ctx) -> void;

		ID3D11VertexShader* vertexShader = nullptr;
		ID3D11PixelShader* pixelShader = nullptr;

		private:

		template<typename T>
		static auto CreateShaderFromBytecode(ID3D11Device1* device, ShaderStage shaderType, const void* shaderBytecode) -> decltype(auto) {
			T* result = nullptr;
			switch (shaderType) {
				case ShaderStage::Vertex: {
					ASSERT_ERROR_RESULT(device->CreateVertexShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11VertexShader**>(&result)));
				} break;

				case ShaderStage::Hull: {
					ASSERT_ERROR_RESULT(device->CreateHullShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11HullShader**>(&result)));
				} break;

				case ShaderStage::Domain: {
					ASSERT_ERROR_RESULT(device->CreateDomainShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11DomainShader**>(&result)));
				} break;

				case ShaderStage::Geometry: {
					ASSERT_ERROR_RESULT(device->CreateGeometryShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&result)));
				} break;

				case ShaderStage::Pixel: {
					ASSERT_ERROR_RESULT(device->CreatePixelShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11PixelShader**>(&result)));
				} break;

				case ShaderStage::Compute: {
					ASSERT_ERROR_RESULT(device->CreateComputeShader(shaderBytecode, sizeof(shaderBytecode), nullptr, reinterpret_cast<ID3D11ComputeShader**>(&result)));
				} break;

				default: Logger::Error("[ShaderProgram]: Unhandled shader stage");
			}

			return result;
		}
	};
}