#pragma once
#include "platform.h"
#include <math.h>

namespace Nickel {
	struct Color8 {
		u8 r, g, b, a;
	};

	struct Vec2 {
		f32 x, y;

		inline auto Hadamard(const Vec2& a, const Vec2& b) -> Vec2 {
			return {
				a.x * b.x,
				a.y * b.y
			};
		}

		inline auto Dot(const Vec2& b) -> f32 {
			return x * b.x + y * b.y;
		}

		inline auto Length() -> f32 {
			return std::sqrt(Dot(*this));
		}

		inline auto Normalize() -> Vec2 {
			const auto length = Length();
			x /= length;
			y /= length;
		}

		static inline auto Lerp(const Vec2& from, const Vec2& to, f32 t) -> Vec2 {
			return from + (to - from) * t;
		}

		template <typename T>
		friend auto operator+(const Vec2& a, const T& b)->Vec2;
		friend auto operator+(const Vec2& a, const Vec2& b) -> Vec2;

		template <typename T>
		friend auto operator-(const Vec2& a, const T& b) -> Vec2;
		friend auto operator-(const Vec2& a, const Vec2& b) -> Vec2;

		template <typename T>
		friend auto operator*(const Vec2& a, const T& b) -> Vec2;

		template <typename T>
		friend auto operator/(const Vec2& a, const T& b) -> Vec2;
		friend auto operator/(const Vec2& a, const Vec2& b) -> Vec2;

		template <typename T>
		friend auto operator+=(const Vec2& a, const T& b);
		friend auto operator+=(const Vec2& a, const Vec2& b);

		template <typename T>
		friend auto operator-=(const Vec2& a, const T& b);
		friend auto operator-=(const Vec2& a, const Vec2& b);

		template <typename T>
		friend auto operator*=(const Vec2& a, const T& b);

		template <typename T>
		friend auto operator/=(const Vec2& a, const T& b);
		friend auto operator/=(const Vec2& a, const Vec2& b);
	};

	struct Vec3 {
		f32 x, y, z;

		inline auto Hadamard(const Vec3& a, const Vec3& b) -> Vec3 {
			return {
				a.x * b.x,
				a.y * b.y,
				a.z * b.z
			};
		}

		inline auto Dot(const Vec3& b) -> f32 {
			return x * b.x + y * b.y + z * b.z;
		}

		auto Cross(const Vec3& a, const Vec3& b) -> Vec3 {
			return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		}

		inline auto Length() -> f32 {
			return std::sqrt(Dot(*this));
		}

		inline auto Normalize() -> Vec3 {
			const auto length = Length();
			x /= length;
			y /= length;
			z /= length;
		}

		static inline auto Lerp(const Vec3& from, const Vec3& to, f32 t) -> Vec3 {
			return from + (to - from) * t;
		}

		template <typename T>
		friend auto operator+(const Vec3& a, const T& b)->Vec3;
		friend auto operator+(const Vec3& a, const Vec3& b)->Vec3;

		template <typename T>
		friend auto operator-(const Vec3& a, const T& b)->Vec3;
		friend auto operator-(const Vec3& a, const Vec3& b)->Vec3;

		template <typename T>
		friend auto operator*(const Vec3& a, const T& b)->Vec3;

		template <typename T>
		friend auto operator/(const Vec3& a, const T& b)->Vec3;
		friend auto operator/(const Vec3& a, const Vec3& b)->Vec3;

		template <typename T>
		friend auto operator+=(Vec3& a, const T& b)->Vec3&;
		friend auto operator+=(Vec3& a, const Vec3& b)->Vec3&;

		template <typename T>
		friend auto operator-=(Vec3& a, const T& b)->Vec3&;
		friend auto operator-=(Vec3& a, const Vec3& b)->Vec3&;

		template <typename T>
		friend auto operator*=(Vec3& a, const T& b)->Vec3&;

		template <typename T>
		friend auto operator/=(Vec3& a, const T& b)->Vec3&;
		friend auto operator/=(Vec3& a, const Vec3& b)->Vec3&;
	};

	struct Vec4 {
		f32 x, y, z, w;
	};

	// Vec2
	template <typename T>
	inline auto operator+(const Vec2& a, const T& b) -> Vec2 { return { a.x + b, a.y + b }; }
	inline auto operator+(const Vec2& a, const Vec2& b) -> Vec2 { return { a.x + b.x, a.y + b.y }; }

	template <typename T>
	inline auto operator-(const Vec2& a, const T& b) -> Vec2 { return { a.x - b, a.y - b }; }
	inline auto operator-(const Vec2& a, const Vec2& b) -> Vec2 { return { a.x - b.x, a.y - b.y }; }

	template <typename T>
	inline auto operator*(const Vec2& a, const T& b) -> Vec2 { return { a.x * b, a.y * b }; }

	template <typename T>
	inline auto operator/(const Vec2& a, const T& b) -> Vec2 { Assert(static_cast<i32>(b) != 0); return { a.x / b, a.y / b }; }
	inline auto operator/(const Vec2& a, const Vec2& b) -> Vec2 { Assert(b.x != 0.0f && b.y != 0.0f); return { a.x / b.x, a.y / b.y }; }

	template <typename T>
	inline auto operator+=(Vec2& a, const T& b) -> Vec2& { a.x += b; a.y += b; return a; }
	inline auto operator+=(Vec2& a, const Vec2& b) -> Vec2& { a.x += b.x; a.y += b.y; return a; }

	template <typename T>
	inline auto operator-=(Vec2& a, const T& b) -> Vec2& { a.x -= b; a.y -= b; return a; }
	inline auto operator-=(Vec2& a, const Vec2& b) -> Vec2& { a.x -= b.x; a.y -= b.y; return a; }

	template <typename T>
	inline auto operator*=(Vec2& a, const T& b) -> Vec2& { a.x *= b; a.y *= b; return a; }

	template <typename T>
	inline auto operator/=(Vec2& a, const T& b) -> Vec2& { Assert(static_cast<i32>(b) != 0); a.x /= b.x; a.y /= b.y; return a; }
	inline auto operator/=(Vec2& a, const Vec2& b) -> Vec2& { Assert(b.x != 0.0f && b.y != 0.0f); a.x /= b.x; a.y /= b.y; return a; }

	// Vec3
	template <typename T>
	inline auto operator+(const Vec3& a, const T& b) -> Vec3 { return { a.x + b, a.y + b, a.z + b}; }
	inline auto operator+(const Vec3& a, const Vec3& b) -> Vec3 { return { a.x + b.x, a.y + b.y, a.z + b.z }; }

	template <typename T>
	inline auto operator-(const Vec3& a, const T& b) -> Vec3 { return { a.x - b, a.y - b, a.z - b }; }
	inline auto operator-(const Vec3& a, const Vec3& b) -> Vec3 { return { a.x - b.x, a.y - b.y, a.z - b.z }; }

	template <typename T>
	inline auto operator*(const Vec3& a, const T& b) -> Vec3 { return { a.x * b, a.y * b, a.z * b }; }

	template <typename T>
	inline auto operator/(const Vec3& a, const T& b) -> Vec3 { Assert(static_cast<i32>(b) != 0); return { a.x / b, a.y / b, a.z / b }; }
	inline auto operator/(const Vec3& a, const Vec3& b) -> Vec3 { Assert(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f); return { a.x / b.x, a.y / b.y, a.z / b.z }; }

	template <typename T>
	inline auto operator+=(Vec3& a, const T& b) -> Vec3& { a.x += b; a.y += b; a.z += b; return a; }
	inline auto operator+=(Vec3& a, const Vec3& b) -> Vec3& { a.x += b.x; a.y += b.y; a.z += b.z; return a; }

	template <typename T>
	inline auto operator-=(Vec3& a, const T& b) -> Vec3& { a.x -= b; a.y -= b; a.z -= b; return a; }
	inline auto operator-=(Vec3& a, const Vec3& b) -> Vec3& { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }

	template <typename T>
	inline auto operator*=(Vec3& a, const T& b) -> Vec3& { a.x *= b; a.y *= b; a.z *= b; return a; }

	template <typename T>
	inline auto operator/=(Vec3& a, const T& b) -> Vec3& { Assert(static_cast<i32>(b) != 0); a.x /= b; a.y /= b; a.z /= b; return a; }
	inline auto operator/=(Vec3& a, const Vec3& b) -> Vec3& { Assert(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f); a.x /= b.x; a.y /= b.y; a.z /= b.z; return a; }
}