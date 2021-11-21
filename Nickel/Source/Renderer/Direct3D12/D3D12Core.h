#pragma once
#include "DirectX12Includes.h"

namespace Nickel::Renderer::DX12Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;
}