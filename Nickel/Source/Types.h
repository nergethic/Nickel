#pragma once

#include <type_traits>
#include <typeinfo>

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

constexpr u64 u64_invalid_id{ 0xffff'ffff'ffff'ffffui64 };
constexpr u32 u32_invalid_id{ 0xffff'ffffui32 };
constexpr u16 u16_invalid_id{ 0xffffui16 };
constexpr u8 u8_invalid_id{ 0xffui8 };

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
		//assert(index != Internal::indexMask);
		return index;
	}

	constexpr auto GetGeneration(IdType id) -> IdType {
		return (id >> Internal::indexBits) & Internal::generationMask;
	}

	constexpr auto NewGeneration(IdType id) -> IdType {
		const IdType generation = Id::GetGeneration(id) + 1;
		//assert(generation < ((static_cast<u64>(1) << Internal::generationBits) - 1));
		return GetIndex(id) | (generation << Internal::indexBits);
	}

#define DEFINE_TYPED_ID(name) using name = Id::IdType;
}