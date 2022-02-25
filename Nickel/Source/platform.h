#pragma once
#include "CommonHeaders.h"
#include <source_location>

#include "Logger.h"
#include "Utilities.h"
#include "Window.h"

#include "imgui/imgui.h";
#include "imgui/imgui_impl_win32.h";
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"

inline auto GetSourceLocation(const std::source_location& loc) -> std::string {
	std::ostringstream result("file: ", std::ios_base::ate);
	result << loc.file_name() << std::endl
		<< "function: '" << loc.function_name() << "'" << std::endl
		<< "line: " << loc.line();

	return result.str();
}

// TODO: change to normal procedure - make a platform universal MessageBox
#if defined(_DEBUG)
#define Assert(expr) {  \
		if (!(expr)) {      \
			MessageBox(nullptr, GetSourceLocation(std::source_location::current()).c_str(), TEXT("Assertion Failed"), MB_OK); \
			__debugbreak(); \
			*(int *)0 = 0;  \
		}                   \
	}
#else
#define Assert(expr) {}
#endif

//struct MemoryPool {
//	size_t base;

//};

// #define PushStruct(s) (size_of(s))
// entity = PushStruct(Arena, Entity);

struct LoadedImageData {
	u8* data;
	u32 width;
	u32 height;
};

template <typename T>
inline auto NoConst(const T& x) -> T& {
	return const_cast<T&>(x);
}

struct Win32State {
	u64 TotalSize;
	void *GameMemoryBlock;
};

typedef struct GameMemory {
	bool32 isInitialized;

	u64 permanentStorageSize;
	void *permanentStorage; // NOTE: REQUIRED to be cleared to zero at startup

	u64 temporaryStorageSize;
	void *temporaryStorage; // NOTE: REQUIRED to be cleared to zero at startup
} GameMemory;

typedef struct GameButtonState {
	int halfTransitionCount;
	bool endedDown;
} GameButtonState;

typedef struct GameControllerInput {
	bool isConnected;
	bool isAnalog;
	bool stickAverageX;
	bool stickAverageY;

	union {
		GameButtonState buttons[12];
		struct {
			GameButtonState up;
			GameButtonState down;
			GameButtonState left;
			GameButtonState right;

			GameButtonState actionUp;
			GameButtonState actionDown;
			GameButtonState actionLeft;
			GameButtonState actionRight;

			GameButtonState leftShoulder;
			GameButtonState rightShoulder;

			GameButtonState back;
			GameButtonState start;

			// NOTE: All buttons must be added above this line
			GameButtonState Count;
		};
	};
} GameControllerInput;

typedef struct GameInput {
	GameButtonState mouseButtons[5];
	i32 mouseX, mouseY, mouseZ;
	f32 normalizedMouseX, normalizedMouseY;

	f32 dt;

	GameControllerInput controllers[2];
} GameInput;