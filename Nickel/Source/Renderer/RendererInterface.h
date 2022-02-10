#pragma once

#include "../Window.h"

namespace Nickel::Renderer {
	DEFINE_TYPED_ID(SurfaceId);
	class Surface {
	public:
		constexpr explicit Surface(u32 id) : id(id) {}
		constexpr Surface() = default;
		constexpr auto GetID() const -> u32 { return id; }
		constexpr auto IsValid() const -> bool { return Id::IsValid(id); }

		auto Resize(u32 width, u32 height) const -> void;
		auto GetWidth() const->u32;
		auto GetHeight() const->u32;
		auto Render() const -> void;
		//auto IsClosed() const -> bool;

	private:
		SurfaceId id{ Id::invalidId };
	};

	enum class GraphicsPlatform : u32 {
		Direct3D11 = 0,
		Direct3D12
	};

	auto Init(GraphicsPlatform platform) -> bool;
	auto Shutdown() -> void;
	auto Render() -> void;

	auto CreateSurface(Platform::Window window)->Surface;
	auto RemoveSurface(SurfaceId id) -> void;
}