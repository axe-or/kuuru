#include "base/memory.h"
#include "base/arena_allocator.h"
#include "base/heap_allocator.h"

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

int main(){
    Mem_Allocator allocator, temp_allocator;
    init_allocators(&allocator, &temp_allocator);
}