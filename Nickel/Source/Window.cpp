#if INCLUDE_WINDOW_CPP
#include "Window.h"
namespace Nickel::Platform {
	auto Window::GetHandle() const -> void* {
		return reinterpret_cast<void*>(GetWindowHandle(id));
	}
}
#endif