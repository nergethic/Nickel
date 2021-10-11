#pragma once
#include <stdint.h>
#include <stddef.h>
#include <utility>

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
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

#define _DEBUG 1;

//struct MemoryPool {
//	size_t base;

//};

// #define PushStruct(s) (size_of(s))
// entity = PushStruct(Arena, Entity);

struct Win32State
{
	u64 TotalSize;
	void *GameMemoryBlock;
};

typedef struct GameMemory
{
	bool32 isInitialized;

	u64 permanentStorageSize;
	void *permanentStorage; // NOTE: REQUIRED to be cleared to zero at startup

	u64 temporaryStorageSize;
	void *temporaryStorage; // NOTE: REQUIRED to be cleared to zero at startup
} GameMemory;

typedef struct GameButtonState
{
	int halfTransitionCount;
	bool endedDown;
} GameButtonState;

typedef struct GameControllerInput
{
	bool isConnected;
	bool isAnalog;
	bool stickAverageX;
	bool stickAverageY;

	union
	{
		GameButtonState buttons[12];
		struct
		{
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

typedef struct GameInput
{
	GameButtonState mouseButtons[5];
	i32 mouseX, mouseY, mouseZ;

	f32 dt;

	GameControllerInput controllers[2];
} GameInput;