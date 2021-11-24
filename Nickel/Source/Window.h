#pragma once
#include "platform.h"

namespace Nickel::Platform {
	DEFINE_TYPED_ID(WindowId);

	class Window {
	public:
		constexpr explicit Window(WindowId id) : id(id) {}
		constexpr Window() : id(Id::invalidId) {};
		constexpr auto GetId() const -> WindowId { return id; };
		constexpr auto IsValid() const -> bool { return Id::IsValid(id); }

		auto SetFullscreen(bool isFullscreen) const -> void;
		auto IsFullscreen() const -> bool;
		inline auto GetHandle() const -> void* { return nullptr; };
		//auto SetCaption(const wchar_t* coption) const -> void;

		//auto Resize(u32 width, u32 height) const -> void;
		inline auto GetWidth() const -> u32 { return 1280; };
		inline auto GetHeight() const -> u32 { return 720; };
		//auto IsClosed() const -> bool;

	private:
		WindowId id = Id::invalidId;
	};
}