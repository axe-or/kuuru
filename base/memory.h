#pragma once

// Interface ///////////////////////////////////////////////////////////////////
#include "prelude.h"
#include "assert.h"

#define New(T_, N_, Al_) mem_alloc((Al_), sizeof(T_) * (N_), alignof(T_))

typedef struct {
	byte* data;
	isize len;
} Bytes;

enum Allocator_Op {
	Mem_Op_Alloc    = 1,
	Mem_Op_Resize   = 2,
	Mem_Op_Free     = 3,
	Mem_Op_Free_All = 4,

	Mem_Op_Query = 0,
};

enum Allocator_Capability {
	Allocator_Alloc_Any = 1 << 0,
	Allocator_Free_Any  = 1 << 1,
	Allocator_Free_All  = 1 << 2,
	Allocator_Resize    = 1 << 3,
	Allocator_Align_Any = 1 << 4,
};

typedef void* (*Mem_Allocator_Func) (
	void* impl,
	enum Allocator_Op op,
	void* old_ptr,
	isize size, isize align,
	i32* capabilities
);

typedef struct {
	Mem_Allocator_Func func;
	void* data;
} Mem_Allocator;

static inline
void mem_set(void* p, byte val, isize nbytes){
	__builtin_memset(p, val, nbytes);
}

static inline
void mem_copy(void* dest, void const * src, isize nbytes){
	__builtin_memmove(dest, src, nbytes);
}

static inline
int mem_valid_alignment(isize align){
	return (align & (align - 1)) == 0 && (align != 0);
}

// Align p to alignment a, this only works if a is a non-zero power of 2
static inline
uintptr align_forward_ptr(uintptr p, uintptr a){
	debug_assert(mem_valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1);
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

// Get capabilities of allocator as a number, you can use bit operators to check it.
i32 allocator_query_capabilites(Mem_Allocator allocator, i32* capabilities);

// Allocate fresh memory, filled with 0s. Returns NULL on failure.
void* mem_alloc(Mem_Allocator allocator, isize size, isize align);

// Re-allocate memory in-place without changing the original pointer. Returns NULL on failure.
void* mem_resize(Mem_Allocator allocator, void* ptr, isize new_size);

// Free pointer to memory, includes alignment information, which is required for
// some allocators, freeing NULL is a no-op
void mem_free_ex(Mem_Allocator allocator, void* p, isize align);

// Free pointer to memory, freeing NULL is a no-op
void mem_free(Mem_Allocator allocator, void* p);

// Free all pointers owned by allocator
void mem_free_all(Mem_Allocator allocator);

// Implementation //////////////////////////////////////////////////////////////
#ifdef BASE_C_IMPLEMENTATION

i32 allocator_query_capabilites(Mem_Allocator allocator, i32* capabilities){
	if(capabilities == NULL){ return 0; }
	allocator.func(allocator.data, Mem_Op_Query, NULL, 0, 0, capabilities);
	return *capabilities;
}

void* mem_alloc(Mem_Allocator allocator, isize size, isize align){
	void* ptr = allocator.func(allocator.data, Mem_Op_Alloc, NULL, size, align, NULL);
	if(ptr != NULL){
		mem_set(ptr, 0, size);
	}
	return ptr;
}

void* mem_resize(Mem_Allocator allocator, void* ptr, isize new_size){
	void* new_ptr = allocator.func(allocator.data, Mem_Op_Resize, ptr, new_size, 0, NULL);
	return new_ptr;
}

void mem_free_ex(Mem_Allocator allocator, void* p, isize align){
	if(p == NULL){ return; }
	allocator.func(allocator.data, Mem_Op_Free, p, 0, align, NULL);
}

void mem_free(Mem_Allocator allocator, void* p){
	mem_free_ex(allocator, p, 0);
}

void mem_free_all(Mem_Allocator allocator){
	allocator.func(allocator.data, Mem_Op_Free_All, NULL, 0, 0, NULL);
}


#endif
