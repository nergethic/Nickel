#pragma once
#include "DirectX11Includes.h"

namespace Nickel::Renderer::DXLayer11::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;
}