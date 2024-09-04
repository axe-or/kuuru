#include "base.hpp"

void* Arena::alloc(isize nbytes, Align align) {
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

void Arena::free_all(){
    offset = 0;
}

Arena Arena::from(Slice<byte> buf){
    Arena a;
    a.data = buf.raw_data();
    a.capacity = buf.size();
    return a;
}

static void* arena_allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
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

Allocator Arena::allocator(){
    return Allocator::from((void*)(this), arena_allocator_func);
}
