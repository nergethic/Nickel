#pragma once
#include <stdint.h>
#include <stddef.h>
#include <utility>
#include <type_traits>
#include <intrin.h>
#include <source_location>
#include <string>
#include <sstream>
#include <span>
#include <variant>
#include <mutex>

#include "Logger.h"

#include "imgui/imgui.h";
#include "imgui/imgui_impl_win32.h";
#include "imgui/imgui_impl_dx11.h"

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


#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 bool32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

//struct MemoryPool {
//	size_t base;

//};

// #define PushStruct(s) (size_of(s))
// entity = PushStruct(Arena, Entity);

namespace Nickel::Id {
	using IdType = u32;

	namespace Internal {
		constexpr u32 generationBits = 8;
		constexpr u32 indexBits = sizeof(IdType) * 8 - generationBits;
		constexpr IdType indexMask = (IdType{ 1 } << indexBits) - 1;
		constexpr IdType generationMask = (IdType{ 1 } << generationBits) - 1;
		
	}

	constexpr IdType invalidId = IdType(-1);
	constexpr u32 minDeletedElements = 1024;

	using GenerationType = std::conditional_t<Internal::generationBits <= 16, std::conditional_t<Internal::generationBits <= 8, u8, u16>, u32>;
	static_assert(sizeof(GenerationType) * 8 >= Internal::generationBits);
	static_assert(sizeof(IdType) - sizeof(GenerationType) > 0);

	constexpr auto IsValid(IdType id) -> bool {
		return id != invalidId;
	}

	constexpr auto GetIndex(IdType id) -> IdType {
		IdType index = id & Internal::indexMask;
		Assert(index != Internal::indexMask);
		return index;
	}

	constexpr auto GetGeneration(IdType id) -> IdType {
		return (id >> Internal::indexBits) & Internal::generationMask;
	}

	constexpr auto NewGeneration(IdType id) -> IdType {
		const IdType generation = Id::GetGeneration(id) + 1;
		assert(generation < ((static_cast<u64>(1) << Internal::generationBits)-1));
		return GetIndex(id) | (generation << Internal::indexBits);
	}

	#define DEFINE_TYPED_ID(name) using name = Id::IdType;
}

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