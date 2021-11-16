#pragma once

#include "renderer.h"

namespace Nickel::Renderer {
	struct PlatformInterface {
		auto (*Init)(void) -> bool;
		auto (*Shutdown)(void) -> void;
	};
}