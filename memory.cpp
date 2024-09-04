#pragma once

#include "assert.cpp"

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

void free_all(Allocator a){
	a.free_all();
}
