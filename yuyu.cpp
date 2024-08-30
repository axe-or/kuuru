// cflags = -O0 -fPIC -pipe -Wall -Wextra
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <new>

#pragma region Prelude
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

template<typename T>
T min(T a, T b){
	return (a < b) ? a : b;
}

template<typename T>
T max(T a, T b){
	return (a > b) ? a : b;
}

template<typename T, typename... Args>
T min(T a, T b, Args&& ...rest){
	if(a < b){
		return min(a, rest...);
	}
	else {
		return min(b, rest...);
	}
}

template<typename T, typename... Args>
T max(T a, T b, Args&& ...rest){
	if(a > b){
		return max(a, rest...);
	}
	else {
		return max(b, rest...);
	}
}

template<typename T>
T clamp(T lo, T x, T hi){
	return min(max(lo, x), hi);
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

#pragma region Debug
#include <iostream> /* DEBUG ONLY. DON'T USE iostream, IT SUCKS. */

void assert(bool cond, cstring msg){
	if(!cond){
		std::fprintf(stderr, "Assert failed: %s\n", msg);
		throw "FAILED ASSERT";
	}
}

[[noreturn]]
void unimplemented(cstring msg = ""){
	std::fprintf(stderr, "Unimplemented code: %s\n", msg);
	throw "UNIMPLEMENTED";
}

[[noreturn]]
void panic(cstring msg){
	std::fprintf(stderr, "Panic: %s\n", msg);
	throw "PANIC";
}

template<typename T>
void print(T x){
	std::cout << x << '\n';
}

template<typename T, typename ...Args>
void print(T x, Args&& ...args){
	std::cout << x << ' ';
	print(args...);
}

template<typename A>
void print_arr(A arr){
	for(isize i = 0; i < arr.size(); i++){
		std::cout << arr[i] << ' ';
	}
	std::cout << '\n';
}

#pragma endregion

#pragma region Memory
using Align = decltype(alignof(int));

void mem_copy(void* dest, void const* src, isize nbytes){
	__builtin_memmove(dest, src, nbytes);
}

void mem_copy_no_overlap(void* dest, void const* src, isize nbytes){
	__builtin_memcpy(dest, src, nbytes);
}

void mem_set(void* dest, byte val, isize nbytes){
	__builtin_memset(dest, val, nbytes);
}

void mem_zero(void* dest, isize nbytes){
	mem_set(dest, 0, nbytes);
}

bool valid_alignment(Align align){
	return (align != 0) && ((align & (align - 1)) == 0);
}

uintptr align_forward(uintptr n, Align align){
	assert(valid_alignment(align), "Invalid memory alignment");
	uintptr mod = n & (uintptr(align) - 1);
	uintptr aligned = n;

	if(mod != 0){
		aligned = n + (align - mod);
	}

	return aligned;
}

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

	void free(void const* ptr){
		fn_(Operation::Free, impl_, 0, 0, ptr, nullptr);
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
	allocator.free(ptr);
}

void free_all(Allocator a){
	a.free_all();
}
#pragma endregion

#pragma region Container
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
	allocator.free(s.raw_data());
}

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
		allocator.free(data);

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

	static DynamicArray create(Allocator allocator){
		DynamicArray arr;
		arr.allocator = allocator;
		return arr;
	}

	void dealloc(){
		clear();
		allocator.free(data);
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
	static void* alloc(isize nbytes, Align align) {
		if(nbytes == 0){ return nullptr; }
		byte* p = new(std::align_val_t(align)) byte[nbytes];
		if(p != nullptr){
			mem_zero(p, nbytes);
		}
		return (void*)(p);
	}

	static void free(void const* ptr) {
		if(ptr == nullptr) { return; }
		delete[] (byte const*)(ptr);
	}

	static void* _allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
		using O = Allocator::Operation;

		(void)impl;

		switch(op){
			case O::Alloc:
				return HeapAllocator::alloc(size, align);
			break;

			case O::Free:
				HeapAllocator::free(ptr);
			break;

			case O::Free_All: break;

			case O::Query:
				using C = Allocator::Capability;
				*query_res = i32(C::Alloc_Any) | i32(C::Free_Any) | i32(C::Align_Any);
			break;
		}

		return nullptr;

	}

	static Allocator get(){
		return Allocator::from(nullptr, _allocator_func);
	}
};

struct Arena {
	byte* data = nullptr;
	uintptr capacity = 0;
	uintptr offset = 0;

	void* alloc(isize nbytes, Align align) {
		if(nbytes == 0){ return nullptr; }

		auto size = uintptr(nbytes);
		auto base = uintptr(data) + offset;
		auto limit = uintptr(data) + capacity;

		auto aligned_base = align_forward(base, align);
		auto padding = aligned_base - base;

		if((aligned_base + size) > limit){
			return nullptr; /* Out of memory */
		}
		offset += padding + size;
		return (void*)(aligned_base);
	}

	void free_all(void) {
		offset = 0;
	}

	static Arena from(Slice<byte> buf){
		Arena a;
		a.data = buf.raw_data();
		a.capacity = buf.size();
		return a;
	}

	static void* _allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
		using O = Allocator::Operation;

		auto self = (Arena*)(impl);
		(void)ptr;

		switch(op){
			case O::Alloc:
				return self->alloc(size, align);
			break;

			case O::Free: break;

			case O::Free_All:
				self->free_all();
			break;

			case O::Query:
				using C = Allocator::Capability;
				*query_res = i32(C::Alloc_Any) | i32(C::Free_All) | i32(C::Align_Any);
			break;
		}

		return nullptr;
	}

	Allocator allocator(){
		return Allocator::from((void*)(this), _allocator_func);
	}
};
#pragma endregion

#pragma region Tests
struct Test {
	cstring title = "";
	i32 failed = 0;
	i32 total = 0;

	void report(){
		cstring msg = (failed == 0) ? "PASS" : "FAIL";
		std::printf("[%s] %s ok in %d/%d\n", title, msg, total - failed, total);
	}

	bool expect(bool pred){
		if(!pred){
			std::printf("Failed expect\n");
			failed += 1;
		}
		total += 1;
		return pred;
	}

	static Test create(cstring title){
		Test t;
		t.title = title;
		return t;
	}

	~Test(){
		report();
	}
};

void test_arena(){
	auto t = Test::create("Arena Allocator");
	auto heap_alloc = HeapAllocator::get();
	auto buf = make_slice<byte>(128, heap_alloc);
	defer(destroy(buf, heap_alloc));
	auto arena = Arena::from(buf);
	auto allocator = arena.allocator();
	{
		auto old_offset = arena.offset;
		(void) make<i32>(allocator);
		t.expect(old_offset + 4 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i8>(allocator);
		t.expect(old_offset + 1 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i32>(allocator);
		t.expect(old_offset + 3 + 4 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i64>(allocator);
		t.expect(old_offset + 4 + 8 == arena.offset);
	}
	{
		auto p = make_slice<byte>(128, allocator);
		t.expect(p.empty());
	}
	free_all(allocator);
	{
		auto p = make_slice<byte>(128, allocator);
		t.expect(!p.empty());
		p[127] = '1';
	}
	{
		auto p = make<i8>(allocator);
		t.expect(p == nullptr);
	}
}

void test_dynamic_array(){
	// TODO, use .expect once we have string formatting
	auto t = Test::create("Dynamic Array");
	auto arr = DynamicArray<i32>::create(HeapAllocator::get());
	defer(destroy(arr));

	constexpr auto print_arr = [](DynamicArray<i32> a){
		print("cap:", a.cap(), "len:", a.size());
		for(isize i = 0; i < a.size(); i += 1){
			std::cout << a[i] << ' ';
		}
		print("");
	};

	print_arr(arr);
	arr.append(6);
	arr.append(9);
	print_arr(arr);
	arr.insert(0, 4);
	arr.insert(1, 2);

	arr.insert(2, 0);

	print_arr(arr);
	arr.remove(arr.size() - 1);
	arr.remove(3);
	print_arr(arr);
	arr.remove(0);
	print_arr(arr);
	arr.remove(0);
	arr.remove(0);
	print_arr(arr);
	arr.insert(0, 69);
	arr.insert(0, 420);
	print_arr(arr);
}
#pragma endregion

#pragma region String
isize cstring_len(cstring cstr){
	constexpr isize MAX_CSTR_LEN = isize(~u32(0) >> u32(1));
	for(isize i = 0; i < MAX_CSTR_LEN; i += 1){
		if(cstr[i] == 0){
			return i;
		}
	}
	return MAX_CSTR_LEN;
}

struct string {
	byte const* data = nullptr;
	isize length = 0;

	// Implicit constructor
	string(cstring cs){
		data = (byte const *)(cs);
		length = cstring_len(cs);
	}

	string(){}

	static string from_bytes(Slice<byte> bytes){
		string s;
		s.data = bytes.raw_data();
		s.length = bytes.size();
		return s;
	}
	
	static string from_cstring(cstring cstr){
		return from_cstring(cstr, cstring_len(cstr));
	}

	static string from_cstring(cstring cstr, isize length){
		string s;
		s.data = (byte const *)(cstr);
		s.length = length;
		return s;
	}
	
	isize size() const { return length; }
	
	byte const* raw_data() const { return data; }
	
	bool operator==(string b){
		if(length != b.length){ return false; }
		
		for(isize i = 0; i < length; i += 1){
			if(data[i] != b.data[i]){ return false; }
		}
		
		return true;
	}
};

 namespace utf8 {
constexpr auto surrogate_range = Array<rune, 2>{0xd800, 0xdfff};
constexpr auto continuation_range = Array<byte, 2>{0x80, 0xbf};

constexpr rune error_rune = 0xfffd;

constexpr rune range_1 = 0x7f;
constexpr rune range_2 = 0x7ff;
constexpr rune range_3 = 0xffff;
constexpr rune range_4 = 0x10ffff;

constexpr rune mask_2 = 0x1f; /* 0001_1111 */
constexpr rune mask_3 = 0x0f; /* 0000_1111 */
constexpr rune mask_4 = 0x07; /* 0000_0111 */

constexpr rune mask_x = 0x3f; /* 0011_1111 */

constexpr rune size_2 = 0xc0; /* 110x_xxxx */
constexpr rune size_3 = 0xe0; /* 1110_xxxx */
constexpr rune size_4 = 0xf0; /* 1111_0xxx */

constexpr rune cont = 0x80; /* 10xx_xxxx */

pair<Array<byte, 4>, i32> encode(rune r){
	// TODO: Prevent invalid codepoints
	int n = 0;
	Array<byte, 4> bytes = {0};

	if(r <= range_1){
		n = 1;
		bytes[0] = r;
	}
	else if(r <= range_2){
		n = 2;
		bytes[0] = size_2 | ((r >> 6) & mask_2);
		bytes[1] = cont   | ((r >> 0) & mask_x);
	}
	else if(r <= range_3){
		n = 3;
		bytes[0] = size_3 | ((r >> 12) & mask_3);
		bytes[1] = cont   | ((r >> 6) & mask_x);
		bytes[2] = cont   | ((r >> 0) & mask_x);
	}
	else if(r <= range_4){
		n = 4;
		bytes[0] = size_4 | ((r >> 18) & mask_4);
		bytes[1] = cont   | ((r >> 12) & mask_x);
		bytes[2] = cont   | ((r >> 6)  & mask_x);
		bytes[3] = cont   | ((r >> 0)  & mask_x);
	}
	return {bytes, n};
}

pair<rune, i32> decode(Slice<byte> s){
	constexpr pair<rune, i32> decode_error = {error_rune, 0};
	if(s.empty()){ return decode_error; }

	auto buf = s.raw_data();
	u8 first = buf[0];

	i32 len = 0;
	rune codepoint = 0;

	if((first & cont) == 0){
		len = 1;
		codepoint |= first;
	}
	else if ((first & ~mask_2) == size_2 && s.size() >= 2){
		len = 2;
		codepoint |= (buf[0] & mask_2) << 6;
		codepoint |= (buf[1] & mask_x) << 0;
	}
	else if ((first & ~mask_3) == size_3 && s.size() >= 2){
		len = 3;
		codepoint |= (buf[0] & mask_3) << 12;
		codepoint |= (buf[1] & mask_x) << 6;
		codepoint |= (buf[2] & mask_x) << 0;
	}
	else if ((first & ~mask_4) == size_4 && s.size() >= 2){
		len = 4;
		codepoint |= (buf[0] & mask_4) << 18;
		codepoint |= (buf[1] & mask_x) << 12;
		codepoint |= (buf[2] & mask_x) << 6;
		codepoint |= (buf[3] & mask_x) << 0;
	}
	else {
		return decode_error;
	}

	// Validation
	if(codepoint >= surrogate_range[0] && codepoint <= surrogate_range[1]){
		return decode_error;
	}
	if(len > 1 && (buf[1] < continuation_range[0] || buf[1] > continuation_range[1])){
		return decode_error;
	}
	if(len > 2 && (buf[2] < continuation_range[0] || buf[2] > continuation_range[1])){
		return decode_error;
	}
	if(len > 3 && (buf[3] < continuation_range[0] || buf[3] > continuation_range[1])){
		return decode_error;
	}

	return {codepoint, len};
}

}
#pragma endregion

void test_utf8(){
	auto t = Test::create("UTF-8");
	const rune codepoints[] = {
		0x0024,  /* $ */
		0x0418,  /* И */
		0xd55c,  /* 한 */
		0x10348, /* 𐍈 */
	};
	const cstring encoded[] = {"$", "И", "한", "𐍈"};

	isize const N = (sizeof(codepoints) / sizeof(rune));

	for(isize i = 0; i < N; i += 1){
		auto [bytes, len] = utf8::encode(codepoints[i]);
		t.expect(len == i + 1);

		auto a = string::from_bytes(bytes.sub(0, len));
		auto b = string::from_cstring(encoded[i]);

		t.expect(a == b);
	}

	for(isize i = 0; i < N; i += 1){
		auto s = Slice<byte>::from((byte *)encoded[i], cstring_len(encoded[i]));
		auto [codepoint, len] = utf8::decode(s);
		t.expect(len == i + 1);

		rune a = codepoints[i];
		rune b = codepoint;
		t.expect(a == b);
	}

}

int main(void) {
	// test_arena();
	// test_dynamic_array();
	test_utf8();
	return 0;
}
