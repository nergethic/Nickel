#include "RendererInterface.h"
#include "RendererPlatformInterface.h"
#include "Direct3D11/D3D11Interface.h"
#include "Direct3D12/D3D12Interface.h"

namespace Nickel::Renderer {
	namespace {
		PlatformInterface gfx{};
		auto SetPlatformInterface(GraphicsPlatform platform) -> bool {
			switch (platform) {
			case GraphicsPlatform::Direct3D11: {
				DX11Layer::GetPlatformInterface(gfx);
			} break;


			case GraphicsPlatform::Direct3D12: {
				DX12Layer::GetPlatformInterface(gfx);
			} break;

			default:
				return false;
			}

			return true;
		}
	}

	auto Init(GraphicsPlatform platform) -> bool {
		Logger::Init(); // TODO: super ugly, figure out where to initialize this
		return SetPlatformInterface(platform) && gfx.Init();
	}

	auto Shutdown() -> void {
		gfx.Shutdown();
	}

	auto CreateSurface(Platform::Window window) -> Surface {
		return gfx.Surface.Create(window);
	}

	auto RemoveSurface(SurfaceId id) -> void {
		Assert(Id::IsValid(id));
		gfx.Surface.Remove(id);
	}

	auto Surface::Resize(u32 width, u32 height) const -> void {
		Assert(IsValid());
		gfx.Surface.Resize(id, width, height);
	}

	auto Surface::GetWidth() const -> u32 {
		Assert(IsValid());
		return gfx.Surface.GetWidth(id);
	}

	auto Surface::GetHeight() const -> u32 {
		Assert(IsValid());
		return gfx.Surface.GetHeight(id);
	}

	auto Surface::Render() const -> void {
		Assert(IsValid());
		return gfx.Surface.Render(id);
	}
}
