struct HeapAllocator {
	static void* alloc(isize nbytes, Align align) {
		if(nbytes == 0){ return nullptr; }
		byte* p = new(std::align_val_t(align)) byte[nbytes];
		if(p != nullptr){
			mem_zero(p, nbytes);
		}
		return (void*)(p);
	}

	static void free(void const* ptr, Align align) {
		if(ptr == nullptr) { return; }

		operator delete[]((void*)(ptr), std::align_val_t(align));
	}

	static void* _allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
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

	static Allocator get(){
		return Allocator::from(nullptr, _allocator_func);
	}
};

struct Arena {
	byte* data = nullptr;
	uintptr capacity = 0;
	uintptr offset = 0;

	void* alloc(isize nbytes, Align align) {
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

	void free_all(void) {
		offset = 0;
	}

	static Arena from(Slice<byte> buf){
		Arena a;
		a.data = buf.raw_data();
		a.capacity = buf.size();
		return a;
	}

	static void* _allocator_func(Allocator::Operation op, void* impl, isize size, Align align, void const* ptr, i32* query_res){
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

	Allocator allocator(){
		return Allocator::from((void*)(this), _allocator_func);
	}
};
