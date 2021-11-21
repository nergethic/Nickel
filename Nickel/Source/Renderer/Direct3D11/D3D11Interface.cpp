#include "../RendererPlatformInterface.h"
#include "D3D11Interface.h"
#include "D3D11Core.h"

namespace Nickel::Renderer::DXLayer11 {
	auto GetPlatformInterface(PlatformInterface& platformInterface) -> void {
		platformInterface.Init = Core::Init;
		platformInterface.Shutdown = Core::Shutdown;
	}
}