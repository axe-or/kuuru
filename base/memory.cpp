#include "base.hpp"

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
