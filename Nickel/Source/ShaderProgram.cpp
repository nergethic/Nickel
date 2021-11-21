#include "ShaderProgram.h"

namespace Nickel::Renderer::DXLayer11 {
	// auto CreateInputLayout(ID3D11Device1* device, std::span<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDesc, std::span<const u8> shaderBytecodeWithInputSignature) -> ID3D11InputLayout*;

	auto ShaderProgram::Create(ID3D11Device1* device, std::span<const u8> vertexShaderBytecode, std::span<const u8> pixelShaderBytecode) -> void {
		Assert(vertexShaderBytecode.data() != nullptr);
		Assert(pixelShaderBytecode.data() != nullptr);

		//inputLayout = CreateInputLayout(device, vertexLayoutDesc, vertexShaderBytecode);
		inputLayout = CreateInputLayoutFromBytecode(device, vertexShaderBytecode);
		CreateShaderFromBytecode(device, vertexShader, vertexShaderBytecode);
		CreateShaderFromBytecode(device, pixelShader, pixelShaderBytecode);
	}

	auto ShaderProgram::Bind(ID3D11DeviceContext1* ctx) -> void {
		Assert(ctx != nullptr);

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

		ctx->IASetInputLayout(inputLayout);
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

	auto ShaderProgram::CreateInputLayoutFromBytecode(ID3D11Device1* device, std::span<const u8> vertexShaderBytecode) -> ID3D11InputLayout* {
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> vertexShaderReflection = nullptr;
		ASSERT_ERROR_RESULT(D3DReflect(vertexShaderBytecode.data(), vertexShaderBytecode.size(), IID_ID3D11ShaderReflection, (void**)vertexShaderReflection.GetAddressOf()));

		D3D11_SHADER_DESC shaderDesc;
		ASSERT_ERROR_RESULT(vertexShaderReflection->GetDesc(&shaderDesc));

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++) {
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

			// fill out input element desc
			auto elementDesc = D3D11_INPUT_ELEMENT_DESC{
				.SemanticName = paramDesc.SemanticName,
				.SemanticIndex = paramDesc.SemanticIndex,
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			};

			// determine DXGI format
			if (paramDesc.Mask == 1) { // offset += 4
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			} else if (paramDesc.Mask <= 3) { // offset += 8
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			} else if (paramDesc.Mask <= 7) { // offset += 12
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			} else if (paramDesc.Mask <= 15) { // offset += 16
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			inputLayoutDesc.push_back(elementDesc);
		}

		ID3D11InputLayout* inputLayout;
		ASSERT_ERROR_RESULT(device->CreateInputLayout(inputLayoutDesc.data(), static_cast<UINT>(inputLayoutDesc.size()), vertexShaderBytecode.data(), vertexShaderBytecode.size_bytes(), &inputLayout));

		return inputLayout;
	}
}