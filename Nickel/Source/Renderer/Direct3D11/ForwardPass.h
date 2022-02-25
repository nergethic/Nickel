#pragma once

#include "DX11Layer.h"

namespace Nickel::Renderer::DX11Layer::MainPass {
	auto Initialize() -> bool;
	auto Shutdown() -> void;

	auto Render() -> void;
}