#include "base/memory.h"
#include "base/heap_allocator.h"
#include "base/arena_allocator.h"
// #include "compiler/kuuru.hpp"
// #include <cstdio>

#define MEBIBYTE (1024ll * 1024ll)

static void init_allocators(Mem_Allocator* temp, Mem_Allocator* allocator){
	#define ARENA_SIZE (4 * MEBIBYTE)

	static bool initialized = false;

	static Mem_Arena arena;
	static byte arena_buf[ARENA_SIZE] = {0};

	if(!initialized){
		arena_init(&arena, arena_buf, ARENA_SIZE);
		*temp = arena_allocator(&arena);
		*allocator = heap_allocator();
	}

	#undef ARENA_SIZE
}

int main(void) {
	Mem_Allocator allocator, temp_allocator;
	init_allocators(&temp_allocator, &allocator);

	// auto src = read_whole_file("main.cpp", allocator);
	// defer(destroy(src, allocator));

	// writeln(string::from_bytes(src));

	mem_free_all(temp_allocator);
    return 0;
}
