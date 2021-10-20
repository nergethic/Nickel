#pragma once
#include "renderer.h"

namespace Nickel::Renderer {
	class ResourceManager {
	public:
		inline static auto Init(ID3D11Device1* _device) -> void {
			device = _device;

			/*
#if (_WIN32_WINNT >= 0x0A00) // _WIN32_WINNT_WIN10
			Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
			if (FAILED(initialize))
				// error
#else
			HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			if (FAILED(hr))
				// error
#endif
			*/

			D3D11_SAMPLER_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));

			D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressU = addressMode;
			desc.AddressV = addressMode;
			desc.AddressW = addressMode;
			desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
			desc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			desc.MaxAnisotropy = 0;
			desc.MaxLOD = FLT_MAX;
			desc.MinLOD = FLT_MIN;
			desc.MipLODBias = 0;

			defaultSamplerState = DXLayer::CreateSamplerState(device, desc);
		}

		// NOTE: supports BMP, JPEG, PNG, TIFF, GIF
		inline static auto LoadTexture(std::wstring path) -> TextureDX11 {
			TextureDX11 newTex{.samplerState = defaultSamplerState };
			ASSERT_ERROR_RESULT(CreateWICTextureFromFile(device, path.c_str(), &newTex.resource, &newTex.srv));
			return newTex;
		}

	private:
		inline static ID3D11Device1* device;
		inline static ID3D11SamplerState* defaultSamplerState;
	};
}