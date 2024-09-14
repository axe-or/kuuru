#include "base.h"

#include "kuuru_c/lexer.h"

#include "kuuru_c/utilities.h"

#define MEBIBYTE (1024ll * 1024ll)
static void init_allocators(Mem_Allocator* allocator, Mem_Allocator* temp_allocator){
    #define ARENA_SIZE (4 * MEBIBYTE)

    static bool initialized = false;
    static byte arena_buf[ARENA_SIZE] = {0};

    static Mem_Arena arena;

    if(!initialized){
        *allocator = heap_allocator();
        arena_init(&arena, arena_buf, ARENA_SIZE);
        *temp_allocator= arena_allocator(&arena);
        initialized = true;
    }

    #undef ARENA_SIZE
}

struct Token_List {
	Token* tokens;
	isize len;
	isize cap;
};

int main(){
    Mem_Allocator allocator, temp_allocator;
    init_allocators(&allocator, &temp_allocator);

	Lexer lexer = lexer_make(str_from("+-*/%"));
	while(1){
		Token tk = lexer_next(&lexer);
		if(tk.kind == Tk_EOF){ break; }
	}

	Bytes_Buffer bb;
	if(!buffer_init(&bb, temp_allocator, 256)){
		return 1;
	}


	Token tokens[] = {{.lexeme = str_from("Hello"), .kind = Tk_Identifier}};
	format_token_list(&bb, tokens, 1);
	byte* b = buffer_bytes(&bb);
	printf("%s\n", b);

	cleanup: {
		mem_free_all(temp_allocator);
	}
    return 0;
}
