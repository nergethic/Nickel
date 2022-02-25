#pragma once

namespace Nickel::Renderer {
	struct PlatformInterface;

	namespace DX11Layer {
		auto GetPlatformInterface(PlatformInterface& platformInterface) -> void;
	}
}