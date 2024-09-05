#include "yuyu.hpp"
#include <cstdio>

void writeln(string msg){
	std::printf("%.*s\n", (int)(msg.size()), msg.raw_data());
}

#define MEBIBYTE (1024ll * 1024ll)

static pair<Allocator> init_allocators(){
	static Allocator allocator{0};
	static Allocator temp_allocator{0};
	static Arena arena{0};
	static Array<byte, 8 * MEBIBYTE> arena_buf{0};

	static bool init = false;
	if(!init){
		mem_zero(arena_buf.sub().raw_data(), arena_buf.size());
		arena = Arena::from(arena_buf.sub());
		temp_allocator = arena.allocator();
		allocator = HeapAllocator::get();
		init = true;
	}
	return {allocator, temp_allocator};
}

int main(void) {
	Allocator allocator, temp_allocator;
	/* Init allocators */ {
		auto allocators = init_allocators();
		allocator = allocators.a;
		temp_allocator = allocators.b;
	}
	defer(free_all(temp_allocator));
	
    return 0;
}