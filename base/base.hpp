// This contains all declarations for the `base` library set.
#pragma once

#pragma region Prelude
#include <stddef.h>
#include <stdint.h>
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
T abs(T x){
	return (x < T(0)) ? -x : x;
}

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

#pragma endregion

#pragma region Assert
void assert(bool cond, cstring msg);

[[noreturn]]
void unimplemented(cstring msg = "");

[[noreturn]]
void panic(cstring msg);

#pragma endregion

#pragma region Memory
using Align = decltype(alignof(int));

void mem_copy(void* dest, void const* src, isize nbytes);

void mem_copy_no_overlap(void* dest, void const* src, isize nbytes);

void mem_set(void* dest, byte val, isize nbytes);

void mem_zero(void* dest, isize nbytes);

bool valid_alignment(Align align);

uintptr align_forward(uintptr n, Align align);

// Allocator interface
struct Allocator {
	enum class Operation : i32 {
		Query = 0,
		Alloc,
		Free,
		Free_All,
	};

	using Func = void* (*)(
		Operation op,
		void* impl,
		isize size,
		Align align,
		void const* ptr,
		i32* query_res);

	enum class Capability : i32 {
		Alloc_Any = 1 << 0, // Supports arbitrary allocation size
		Free_Any  = 1 << 1, // Supports individual/out-of-order free
		Free_All  = 1 << 2, // Supports free all
		Align_Any = 1 << 3, // Supports arbitrary alignment
	};

	Allocator::Func fn_ = nullptr;
	void* impl_ = nullptr;

	void* alloc(isize nbytes, Align align){
		return fn_(Operation::Alloc, impl_, nbytes, align, nullptr, nullptr);
	}

	void free(void const* ptr, Align align){
		fn_(Operation::Free, impl_, 0, align, ptr, nullptr);
	}

	void free_all(void){
		fn_(Operation::Free_All, impl_, 0, 0, nullptr, nullptr);
	}

	i32 capabilities(void){
		i32 cap = 0;
		fn_(Operation::Query, impl_, 0, 0, nullptr, &cap);
		return cap;
	}

	static Allocator from(void* impl, Allocator::Func fn){
		Allocator a;
		a.fn_ = fn;
		a.impl_ = impl;
		return a;
	}
};

template<typename T> [[nodiscard]]
T* make(Allocator allocator){
	T* ptr = (T*)(allocator.alloc(sizeof(T), alignof(T)));
	if(ptr != nullptr){
		mem_zero(ptr, sizeof(T));
	}
	return ptr;
}

template<typename T>
void destroy(T* ptr, Allocator allocator){
	if(ptr == nullptr){ return; }
	ptr->~T();
	allocator.free(ptr, alignof(T));
}

[[maybe_unused]]
static void free_all(Allocator a){
	a.free_all();
}

#pragma endregion

#pragma region Containers
template<typename T>
struct Slice {
private:
	T* data = nullptr;
	isize length = 0;

public:
	T& operator[](isize idx){
		assert(idx >= 0 && idx < length, "Out of bounds access to slice");
		return data[idx];
	}

	T const& operator[](isize idx) const {
		assert(idx >= 0 && idx < length, "Out of bounds access to slice");
		return data[idx];
	}

	T at_or(isize idx, T const& val) const {
		if(idx < 0 && idx >= length){
			return val;
		}
		return data[idx];
	}

	Slice<T> sub(isize start, isize end){
		assert(start <= end && start >= 0 && end >= 0, "Invalid range for slice");
		return Slice<T>::from(&data[start], end - start);
	}

	Slice<T> sub(){
		return *this;
	}

	void reset() {
		data = nullptr;
		length = 0;
	}

	bool empty() const {
		return (length == 0) || (data == nullptr);
	}

	isize size() const { return length; }

	T* raw_data() const { return data; }

	static Slice<T> from(T* ptr, isize n){
        assert(n >= 0, "Invalid size");
		Slice<T> s;
		s.data = ptr;
		s.length = n;
		return s;
	}
};

template<typename T> [[nodiscard]]
static Slice<T> make_slice(isize n, Allocator allocator){
	T* elems = (T*)(allocator.alloc(n * sizeof(T), alignof(T)));
	if(elems != nullptr){
		mem_zero(elems, n * sizeof(T));
	}
	return Slice<T>::from(elems, n);
}

template<typename T>
void destroy(Slice<T> s, Allocator allocator){
	for(isize i = 0; i < s.size(); i += 1){
		s[i].~T();
	}
	allocator.free(s.raw_data(), alignof(T));
}

template<typename T>
void reverse(Slice<T> s){
    T* buf = s.raw_data();
    isize n = s.size();

    for(isize i = 0; i < (n / 2); i += 1){
        swap(&buf[i], &buf[n - (i + 1)]);
    }
}

template<typename T>
struct Option {
	union {
		T data;
	};
	bool has_value = false;

	T get(){
		if(!has_value){
			panic("Cannot get() from empty option");
		}
		return data;
	}

	T get_or(T const& v){
		if(!has_value){
			return v;
		}
		return data;
	}

	bool ok() const {
		return has_value;
	}

	void operator=(T const& e){
		set(e);
	}

	void operator=(Option<T> const& opt){
		if(opt.has_value){
			set(opt.data);
		}
	}

	static Option from(T const& e){
		Option<T> opt;
		opt.set(e);
		return opt;
	}

	static constexpr Option<T> none = {0};

	void set(T const& e){
		if(has_value){
			data = e;
		} else {
			new (&data) T(e);
		}
		has_value = true;
	}

	void reset(){
		if(has_value){
			data.~T();
		}
		has_value = false;
	}

	Option(){}

	// Required by C++ due to union
	Option(Option<T> const& opt) : data(opt.data), has_value(opt.has_value) {}

	~Option(){
		set();
	}
};

template<typename T, isize N>
struct Array {
    T data[N];

    T& operator[](isize idx){
   		assert(idx >= 0 && idx < N, "Out of bounds access to array");
        return data[idx];
    }

    T const& operator[](isize idx) const {
   		assert(idx >= 0 && idx < N, "Out of bounds access to array");
        return data[idx];
    }

    isize size() const {
        return N;
    }

	Slice<T> sub(){
		return Slice<T>::from(data, N);
	}

    Slice<T> sub(isize start, isize end){
        return Slice<T>::from(&data[start], end - start);
    }
};

/* Auto generated array operations */ namespace {
	template<typename T, isize N> constexpr auto operator+(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] + b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator-(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] - b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator*(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] * b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator/(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] / b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator%(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] % b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator&(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] & b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator|(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] | b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator^(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] ^ b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<<(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] << b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>>(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] >> b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator&&(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] && b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator||(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] || b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator==(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] == b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator!=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] != b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] > b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] >= b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] < b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] <= b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator+(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = + a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator-(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = - a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator~(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = ~ a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator!(Array<T, N> const& a){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = ! a.data[i]; } return c; }
}

template<typename T>
struct DynamicArray {
	T* data = nullptr;
	isize capacity = 0;
	isize length = 0;
	Allocator allocator = {0};

	void resize(isize new_cap){
		T* new_data = (T*)(allocator.alloc(sizeof(T) * new_cap, alignof(T)));
		assert(new_data != nullptr, "Failed allocation");

		if(data != nullptr){
			mem_copy(new_data, data, sizeof(T) * length);
		}

		if(new_cap > length){
			mem_zero(&new_data[length], (new_cap - length) * sizeof(T));
		}
		allocator.free(data, alignof(T));

		length = min(new_cap, length);
		capacity = new_cap;
		data = new_data;
	}

	void append(T const& e){
		if(length >= capacity){
			resize(align_forward((length * 2) + 1, alignof(T)));
		}

		data[length] = e;
		length += 1;
	}

	void append_slice(Slice<T> elems){
		if(length >= capacity){
			resize(align_forward((length * 2) + elems.size(), alignof(T)));
		}

		mem_copy(&data[length], elems.raw_data(), elems.size() * sizeof(T));
		length += elems.size();
	}

	void pop(){
		if(length == 0){ return; }
		length -= 1;
		data[length].~T();
	}


	void insert(isize idx, T const& e){
		assert(idx >= 0 && idx <= length, "Out of bounds insertion to dynamic array");
		if(idx == length){
			return append(e);
		}

		if(length >= capacity){
			resize(align_forward((length * 2) + 1, alignof(T)));
		}

		mem_copy(&data[idx + 1], &data[idx], sizeof(T) * (length - idx));
		length += 1;
		data[idx] = e;
	}


	void remove(isize idx){
		assert(idx >= 0 && idx < length, "Out of bounds insertion to dynamic array");
		data[idx].~T();

		mem_copy(&data[idx], &data[idx + 1], sizeof(T) * (length - idx));
		length -= 1;
	}

	void clear(){
		for(isize i = 0; i < length; i += 1){
			isize pos = length - (i + 1);
			data[pos].~T();
		}
		length = 0;
	}

	T& operator[](isize idx){
		assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	T const& operator[](isize idx) const {
		assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	isize size() const {
		return length;
	}

	isize cap() const {
		return capacity;
	}

	// Create a clone of the array's current items using allocator.
	Slice<T> build_slice(Allocator allocator){
		auto s = make_slice<T>(allocator);
		mem_copy_no_overlap(s.raw_data(), data, length * sizeof(T));
		return s;
	}

	// Returns elements at moment of extraction, this clears the array.
	Slice<T> extract_slice(){
		auto s = Slice<T>::from(data, length);
		data = nullptr;
		length = 0;
		capacity = 0;
		return s;
	}

	Slice<T> sub(isize start, isize end){
		auto s = Slice<T>::from(data, length).sub(start, end);
		return s;
	}

	static DynamicArray create(Allocator allocator){
		DynamicArray arr;
		arr.allocator = allocator;
		return arr;
	}

	void dealloc(){
		clear();
		allocator.free(data, alignof(T));
		data = nullptr;
		capacity = 0;
	}
};

template<typename T>
void destroy(DynamicArray<T> arr){
	arr.dealloc();
}
#pragma endregion

#pragma region Allocators
struct HeapAllocator {
    static Allocator get();
};

struct Arena {
	byte* data = nullptr;
	uintptr capacity = 0;
	uintptr offset = 0;

    void* alloc(isize nbytes, Align align);
    void free_all(void);
    static Arena from(Slice<byte> buf);
    Allocator allocator();
};

#pragma endregion

#pragma region String 
isize cstring_len(cstring cstr);

namespace utf8 {

constexpr rune error_rune = 0xfffd;

pair<Array<byte, 4>, i32> encode(rune r);
pair<rune, i32> decode(Slice<byte> s);

struct Iterator {
	Slice<byte> data;
	isize current = 0;

    pair<rune, i32> next();
    bool done() const;
};
} /* namespace utf8 */

namespace cpp {
struct UTF8IteratorWrapper : public utf8::Iterator {
	void operator++(){}

	auto operator*(){
		return next();
	}

	bool operator!=(UTF8IteratorWrapper rhs){
		return current != rhs.current;
	}
};
} /* namespace cpp */

struct string {
private:
	byte* data = nullptr;
	isize length = 0;

public:
	static string from_cstring(cstring cstr, isize length);
	static string from_cstring(cstring cstr);
	static string from_bytes(Slice<byte> bytes);
    
    auto size() { return length; }
	auto raw_data() { return data; }
    
	utf8::Iterator iter();

    // Implicit constructor
	string(cstring cs){
        *this = from_cstring(cs);
	}
	string(){}

    auto begin() const {
        cpp::UTF8IteratorWrapper it;
        it.data = Slice<byte>::from((byte*)(data), length);
        return it;
    }   

    auto end() const {
        cpp::UTF8IteratorWrapper it;
        it.current = length;
        return it;
    }
};

struct StringBuilder {
	DynamicArray<byte> data;

    static StringBuilder create(Allocator allocator);

	auto allocator() const { return data.allocator; }
	auto clear(){ data.clear(); }  
	auto dealloc(){	data.dealloc();	}

    void push_rune(rune r);
    void push_string(string s);
    void push_integer(i64 value, i8 base);
    string build();
};

[[maybe_unused]]
static void destroy(StringBuilder sb){
	sb.dealloc();
}

static inline bool operator==(string a, string b){
	if(a.size() != b.size()){ return false; }
    
    auto buf_a = a.raw_data();
    auto buf_b = b.raw_data();

	for(isize i = 0; i < a.size(); i += 1){
		if(buf_a[i] != buf_b[i]){ return false; }
	}
	return true;
}

#pragma endregion


