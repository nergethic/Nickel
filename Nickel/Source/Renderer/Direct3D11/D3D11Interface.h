#pragma once

namespace Nickel::Renderer {
	struct PlatformInterface;

	namespace DXLayer {
		auto GetPlatformInterface(PlatformInterface& platformInterface) -> void;
	}
}