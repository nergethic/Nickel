#pragma once

#include "renderer.h"

namespace Nickel::Renderer {
	struct PlatformInterface {
		auto (*Init)(void) -> bool;
		auto (*Shutdown)(void) -> void;
		auto (*Render)(void) -> void;

		struct {
			auto (*Create)(Platform::Window) -> Surface;
			auto (*Remove)(SurfaceId) -> void;
			auto (*Resize)(SurfaceId, u32, u32) -> void;
			auto (*GetWidth)(SurfaceId) -> u32;
			auto (*GetHeight)(SurfaceId) -> u32;
			auto (*Render)(SurfaceId) -> void;
		} Surface;
	};
}