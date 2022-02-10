#include "../RendererPlatformInterface.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"

namespace Nickel::Renderer::DX12Layer {
	auto GetPlatformInterface(PlatformInterface& platformInterface) -> void {
		platformInterface.Init     = Core::Init;
		platformInterface.Shutdown = Core::Shutdown;

		platformInterface.Surface.Create    = Core::CreateSurface;
		platformInterface.Surface.Remove    = Core::RemoveSurface;
		platformInterface.Surface.Resize    = Core::ResizeSurface;
		platformInterface.Surface.GetWidth  = Core::GetSurfaceWidth;
		platformInterface.Surface.GetHeight = Core::GetSurfaceHeight;
		platformInterface.Surface.Render    = Core::RenderSurface;
	}
}