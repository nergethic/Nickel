#pragma once

namespace Nickel::Renderer {
	struct PlatformInterface;

	namespace DX12Layer {
		auto GetPlatformInterface(PlatformInterface& platformInterface) -> void;
	}
}