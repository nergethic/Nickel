#include "../RendererPlatformInterface.h"
#include "D3D11Interface.h"
#include "D3D11Core.h"

namespace Nickel::Renderer::DX11Layer {
	auto GetPlatformInterface(PlatformInterface& platformInterface) -> void {
		platformInterface.Init = Core::Init;
		platformInterface.Shutdown = Core::Shutdown;

		platformInterface.Surface.Create = Core::CreateSurface;
		platformInterface.Surface.Remove = Core::RemoveSurface;
		platformInterface.Surface.Resize = Core::ResizeSurface;
		platformInterface.Surface.GetWidth = Core::GetSurfaceWidth;
		platformInterface.Surface.GetHeight = Core::GetSurfaceHeight;
		platformInterface.Surface.Render = Core::RenderSurface;
	}
}