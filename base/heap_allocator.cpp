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

static void* allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
    using O = Allocator::Operation;

    (void)impl;

    switch(op){
        case O::Alloc:
            return HeapAllocator::alloc(size, align);
        break;

        case O::Free:
            HeapAllocator::free(ptr, align);
        break;

        case O::Free_All: break;

        case O::Query:
            using C = Allocator::Capability;
            *query_res = i32(C::Alloc_Any) | i32(C::Free_Any) | i32(C::Align_Any);
        break;
    }

    return nullptr;

}

static Allocator HeapAllocator::get(){
    return Allocator::from(nullptr, allocator_func);
}


static void* alloc(isize nbytes, Align align) {
static void free(void const* ptr, Align align) {

static Allocator get(){
