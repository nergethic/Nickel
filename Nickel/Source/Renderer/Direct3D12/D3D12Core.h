#pragma once
#include "../DirectXIncludes.h"

namespace Nickel::Renderer::DX12Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;
}