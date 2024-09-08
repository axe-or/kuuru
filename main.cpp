#include "base/base.hpp"
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

Slice<byte> read_whole_file(string path, Allocator allocator){
	constexpr isize MAX_PATH_SIZE = 4096;
	char namebuf[MAX_PATH_SIZE];

	if(path.size() >= MAX_PATH_SIZE){
		return {};
	}

	mem_copy_no_overlap(namebuf, path.raw_data(), path.size());
	namebuf[path.size()] = 0;

	FILE* f = std::fopen(&namebuf[0], "rb");
	if(f == nullptr){
		return {};
	}
	defer(std::fclose(f));

	isize start = 0;
	isize end = 0;

	std::fseek(f, 0, SEEK_END);
	end = std::ftell(f);
	std::rewind(f);
	start = std::ftell(f);

	auto filedata = make_slice<byte>(end - start, allocator);
	std::fread(filedata.raw_data(), 1, end - start, f);
	return filedata;
}

int main(void) {
	Allocator allocator, temp_allocator;
	/* Init allocators */ {
		auto allocators = init_allocators();
		allocator = allocators.a;
		temp_allocator = allocators.b;
	}
	defer(free_all(temp_allocator));

	auto src = read_whole_file("main.cpp", allocator);
	defer(destroy(src, allocator));

	writeln(string::from_bytes(src));

    return 0;
}
