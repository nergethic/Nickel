#include "../RendererPlatformInterface.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"

namespace Nickel::Renderer::DX12Layer {
	auto GetPlatformInterface(PlatformInterface& platformInterface) -> void {
		platformInterface.Init     = Core::Init;
		platformInterface.Shutdown = Core::Shutdown;
		platformInterface.Render   = Core::Render;
	}
}