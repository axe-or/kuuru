#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <new>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using isize = ptrdiff_t;
using usize = size_t;

using byte = uint8_t;
using rune = int32_t;

using uintptr = uintptr_t;

using cstring = char const *;

static_assert(sizeof(isize) == sizeof(usize), "Mismatched size types");
static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad floating point size types");

template<typename A, typename B = A>
struct pair {
	A a;
	B b;
};

template<typename T> constexpr
T min(T a, T b){
	return (a < b) ? a : b;
}

template<typename T> constexpr
T max(T a, T b){
	return (a > b) ? a : b;
}

template<typename T, typename... Args> constexpr
T min(T a, T b, Args&& ...rest){
	if(a < b){
		return min(a, rest...);
	}
	else {
		return min(b, rest...);
	}
}

template<typename T, typename... Args> constexpr
T max(T a, T b, Args&& ...rest){
	if(a > b){
		return max(a, rest...);
	}
	else {
		return max(b, rest...);
	}
}

template<typename T> constexpr
T clamp(T lo, T x, T hi){
	return min(max(lo, x), hi);
}

template<typename T> constexpr
void swap(T* a, T* b){
	T tmp = *a;
	*a = *b;
	*b = tmp;
}

namespace _defer_impl {
	template<typename F>
	struct Deferred {
		F f;
		Deferred(F&& f) : f(f) {}
		~Deferred(){ f(); }
	};

	#define CONCAT_0(X, Y) X##Y
	
	#define CONCAT_1(X, Y) CONCAT_0(X, Y)
	
	#define CONCAT_COUNTER(X) CONCAT_1(X, __COUNTER__)
	
	#define defer(EXPR) auto CONCAT_COUNTER(_defer_stmt) = \
		::_defer_impl::Deferred([&](){ do { EXPR ; } while(0); });
}

#define prohibit_copy(T) \
	T(T const&) = delete; \
	void operator=(T const&) = delete;

#define prohibit_move(T) \
	T(T &&) = delete; \
	void operator=(T &&) = delete;
