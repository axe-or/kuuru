// cflags = -O0 -fPIC -pipe -Wall -Wextra
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <new>


#pragma region Memory
#pragma endregion

#pragma region Allocator
#pragma endregion

#include "string.cpp"

struct Lexer {
	string source = "";
	utf8::Iterator iter;

	auto advance(){
		return iter.next();
	}

	auto done(){
		return iter.done();
	}

	static Lexer create(string source){
		Lexer lex;
		lex.source = source;
		lex.iter = source.iter();
		return lex;
	}
};

void writeln(string msg){
	std::printf("%.*s\n", (int)(msg.size()), msg.raw_data());
}

#define MEBIBYTE (1024ll * 1024ll)

int main(void) {
	auto arena_buf = make_slice<byte>(8 * MEBIBYTE, HeapAllocator::get());
    defer(destroy(arena_buf, HeapAllocator::get()));
	auto arena = Arena::from(arena_buf);
	auto allocator = arena.allocator();
	defer(free_all(allocator));

	auto sb = StringBuilder::create(allocator);
    int n = 254;
    sb.push_integer(n, 2);
    sb.push_rune('\n');
    sb.push_integer(n, 8);
    sb.push_rune('\n');
    sb.push_integer(n, 10);
    sb.push_rune('\n');
    sb.push_integer(n, 16);
    sb.push_rune('\n');
    writeln(sb.build());
    return 0;
}