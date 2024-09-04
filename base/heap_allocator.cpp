#include "base.hpp"

static void* heap_alloc(isize nbytes, Align align) {
    if(nbytes == 0){ return nullptr; }
    byte* p = new(std::align_val_t(align)) byte[nbytes];
    if(p != nullptr){
        mem_zero(p, nbytes);
    }
    return (void*)(p);
}

static void heap_free(void const* ptr, Align align) {
    if(ptr == nullptr) { return; }

    operator delete[]((void*)(ptr), std::align_val_t(align));
}

static void* heap_allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
    using O = Allocator::Operation;

    (void)impl;

    switch(op){
        case O::Alloc:
            return heap_alloc(size, align);
        break;

        case O::Free:
            heap_free(ptr, align);
        break;

        case O::Free_All: break;

        case O::Query:
            using C = Allocator::Capability;
            *query_res = i32(C::Alloc_Any) | i32(C::Free_Any) | i32(C::Align_Any);
        break;
    }

    return nullptr;

}

Allocator HeapAllocator::get(){
    return Allocator::from(nullptr, heap_allocator_func);
}
