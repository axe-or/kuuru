
#pragma region Tests
struct Test {
	cstring title = "";
	i32 failed = 0;
	i32 total = 0;

	void report(){
		cstring msg = (failed == 0) ? "PASS" : "FAIL";
		std::printf("[%s] %s ok in %d/%d\n", title, msg, total - failed, total);
	}

	bool expect(bool pred){
		if(!pred){
			std::printf("Failed expect\n");
			failed += 1;
		}
		total += 1;
		return pred;
	}

	static Test create(cstring title){
		Test t;
		t.title = title;
		return t;
	}

	~Test(){
		report();
	}
};

void test_arena(){
	auto t = Test::create("Arena Allocator");
	auto heap_alloc = HeapAllocator::get();
	auto buf = make_slice<byte>(128, heap_alloc);
	defer(destroy(buf, heap_alloc));
	auto arena = Arena::from(buf);
	auto allocator = arena.allocator();
	{
		auto old_offset = arena.offset;
		(void) make<i32>(allocator);
		t.expect(old_offset + 4 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i8>(allocator);
		t.expect(old_offset + 1 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i32>(allocator);
		t.expect(old_offset + 3 + 4 == arena.offset);
	}
	{
		auto old_offset = arena.offset;
		(void) make<i64>(allocator);
		t.expect(old_offset + 4 + 8 == arena.offset);
	}
	{
		auto p = make_slice<byte>(128, allocator);
		t.expect(p.empty());
	}
	free_all(allocator);
	{
		auto p = make_slice<byte>(128, allocator);
		t.expect(!p.empty());
		p[127] = '1';
	}
	{
		auto p = make<i8>(allocator);
		t.expect(p == nullptr);
	}
}

void test_dynamic_array(){
	// TODO, use .expect once we have string formatting
	auto t = Test::create("Dynamic Array");
	auto arr = DynamicArray<i32>::create(HeapAllocator::get());
	defer(destroy(arr));

	constexpr auto print_arr = [](DynamicArray<i32> a){
		print("cap:", a.cap(), "len:", a.size());
		for(isize i = 0; i < a.size(); i += 1){
			std::cout << a[i] << ' ';
		}
		print("");
	};

	print_arr(arr);
	arr.append(6);
	arr.append(9);
	print_arr(arr);
	arr.insert(0, 4);
	arr.insert(1, 2);

	arr.insert(2, 0);

	print_arr(arr);
	arr.remove(arr.size() - 1);
	arr.remove(3);
	print_arr(arr);
	arr.remove(0);
	print_arr(arr);
	arr.remove(0);
	arr.remove(0);
	print_arr(arr);
	arr.insert(0, 69);
	arr.insert(0, 420);
	print_arr(arr);
}

void test_utf8(){
	auto t = Test::create("UTF-8");
	const rune codepoints[] = {
		0x0024,  /* $ */
		0x0418,  /* И */
		0xd55c,  /* 한 */
		0x10348, /* 𐍈 */
	};
	const cstring encoded[] = {"$", "И", "한", "𐍈"};

	isize const N = (sizeof(codepoints) / sizeof(rune));

	for(isize i = 0; i < N; i += 1){
		auto [bytes, len] = utf8::encode(codepoints[i]);
		t.expect(len == i + 1);

		auto a = string::from_bytes(bytes.sub(0, len));
		auto b = string::from_cstring(encoded[i]);

		t.expect(a == b);
	}

	for(isize i = 0; i < N; i += 1){
		auto s = Slice<byte>::from((byte *)encoded[i], cstring_len(encoded[i]));
		auto [codepoint, len] = utf8::decode(s);
		t.expect(len == i + 1);

		rune a = codepoints[i];
		rune b = codepoint;
		t.expect(a == b);
	}

}
#pragma endregion
