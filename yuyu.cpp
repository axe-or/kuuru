// cflags = -O0 -fPIC -pipe -Wall -Wextra
#include <cstdio>
#include <cstddef>
#include <cstdint>
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

using isize = ptrdiff_t;
using usize = size_t;

using byte = uint8_t;
using rune = int32_t;

using uintptr = uintptr_t;

using cstring = char const *;

static_assert(sizeof(isize) == sizeof(usize), "Mismatched size types");

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

template<typename T>
void print(T x){
    std::cout << x << '\n';
}

template<typename T, typename ...Args>
void print(T x, Args&& ...args){
    std::cout << x << ' ';
    print(args...);
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

template<typename T>
struct DynamicArray {
private:
    T* data = nullptr;
    isize capacity = 0;
    isize length = 0;
	Allocator allocator = {0};

public:
    void resize(isize new_cap){
        T* new_data = allocator.alloc(sizeof(T) * new_cap, alignof(T));
		assert(new_data != nullptr, "Failed allocation");
        isize nbytes = sizeof(T) * new_cap;

		if(data != nullptr){
			mem_copy(new_data, data, nbytes);
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
    }

    T& operator[](isize idx){
        assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
        return data[idx];
    }

    T const& operator[](isize idx) const {
        assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
        return data[idx];
    }

	Slice<T> extract(){
		resize(length);
		auto s = Slice<T>::from(data, length);
		data = nullptr;
		length = 0;
	}
	
	static DynamicArray create(Allocator allocator){
		DynamicArray arr;
		arr.allocator = allocator;
		return arr;
	}
};

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
#pragma endregion

void test_dynamic_array(){
	auto t = Test::create("Dynamic Array");
	auto arr = DynamicArray<i32>::create(HeapAllocator::get());
	
}

int main(void) {
	test_arena();
    return 0;
}
