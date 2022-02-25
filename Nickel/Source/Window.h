#pragma once
#include "Types.h"

namespace Nickel::Platform {
	static u32 GLOBAL_WINDOW_WIDTH = 1280;
	static u32 GLOBAL_WINDOW_HEIGHT = 720;

	DEFINE_TYPED_ID(WindowId);

	class Window {
	public:
		constexpr explicit Window(WindowId id) : id(id) {}
		constexpr Window() : id(Id::invalidId) {};
		constexpr auto GetId() const -> WindowId { return id; };
		constexpr auto IsValid() const -> bool { return Id::IsValid(id); }

		auto SetFullscreen(bool isFullscreen) const -> void;
		auto IsFullscreen() const -> bool;
		auto GetHandle() const -> void*;
		//auto SetCaption(const wchar_t* coption) const -> void;

		//auto Resize(u32 width, u32 height) const -> void;
		inline auto GetWidth() const -> u32 { return GLOBAL_WINDOW_WIDTH; };
		inline auto GetHeight() const -> u32 { return GLOBAL_WINDOW_HEIGHT; };

		//auto IsClosed() const -> bool;

	private:
		WindowId id = Id::invalidId;
	};
}