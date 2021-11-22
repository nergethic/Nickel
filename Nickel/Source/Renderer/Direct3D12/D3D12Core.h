#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;
	auto Render() -> void;

	auto GetDevice() -> ID3D12Device *const;
}