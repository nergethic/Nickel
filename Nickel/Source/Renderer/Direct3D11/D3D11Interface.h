#pragma once

namespace Nickel::Renderer {
	struct PlatformInterface;

	namespace DXLayer11 {
		auto GetPlatformInterface(PlatformInterface& platformInterface) -> void;
	}
}