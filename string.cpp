#pragma once

#include "prelude.hpp"
#include "container.cpp"

isize cstring_len(cstring cstr){
	constexpr isize MAX_CSTR_LEN = isize(~u32(0) >> u32(1));
	for(isize i = 0; i < MAX_CSTR_LEN; i += 1){
		if(cstr[i] == 0){
			return i;
		}
	}
	return MAX_CSTR_LEN;
}

namespace utf8 {
constexpr Array<rune, 2> surrogate_range = {0xd800, 0xdfff};
constexpr Array<byte, 2> continuation_range = {0x80, 0xbf};

constexpr rune error_rune = 0xfffd;
constexpr Array<byte, 4> error_rune_encoded = {0xef, 0xbf, 0xbd, 0x00};

constexpr rune range_1 = 0x7f;
constexpr rune range_2 = 0x7ff;
constexpr rune range_3 = 0xffff;
constexpr rune range_4 = 0x10ffff;

constexpr rune mask_2 = 0x1f; /* 0001_1111 */
constexpr rune mask_3 = 0x0f; /* 0000_1111 */
constexpr rune mask_4 = 0x07; /* 0000_0111 */

constexpr rune mask_x = 0x3f; /* 0011_1111 */

constexpr rune size_2 = 0xc0; /* 110x_xxxx */
constexpr rune size_3 = 0xe0; /* 1110_xxxx */
constexpr rune size_4 = 0xf0; /* 1111_0xxx */

constexpr rune cont = 0x80; /* 10xx_xxxx */

bool is_utf16_surrogate(rune r){
	return r >= surrogate_range[0] && r <= surrogate_range[1];
}

pair<Array<byte, 4>, i32> encode(rune r){
	int n = 0;
	Array<byte, 4> bytes = {0};

	if(is_utf16_surrogate(r) || r < 0 || r > range_4){
		return {error_rune_encoded, 0};
	}

	if(r <= range_1){
		n = 1;
		bytes[0] = r;
	}
	else if(r <= range_2){
		n = 2;
		bytes[0] = size_2 | ((r >> 6) & mask_2);
		bytes[1] = cont   | ((r >> 0) & mask_x);
	}
	else if(r <= range_3){
		n = 3;
		bytes[0] = size_3 | ((r >> 12) & mask_3);
		bytes[1] = cont   | ((r >> 6) & mask_x);
		bytes[2] = cont   | ((r >> 0) & mask_x);
	}
	else if(r <= range_4){
		n = 4;
		bytes[0] = size_4 | ((r >> 18) & mask_4);
		bytes[1] = cont   | ((r >> 12) & mask_x);
		bytes[2] = cont   | ((r >> 6)  & mask_x);
		bytes[3] = cont   | ((r >> 0)  & mask_x);
	}
	return {bytes, n};
}

pair<rune, i32> decode(Slice<byte> s){
	constexpr pair<rune, i32> decode_error = {error_rune, 0};
	if(s.empty()){ return decode_error; }

	auto buf = s.raw_data();
	u8 first = buf[0];

	i32 len = 0;
	rune codepoint = 0;

	if((first & cont) == 0){
		len = 1;
		codepoint |= first;
	}
	else if ((first & ~mask_2) == size_2 && s.size() >= 2){
		len = 2;
		codepoint |= (buf[0] & mask_2) << 6;
		codepoint |= (buf[1] & mask_x) << 0;
	}
	else if ((first & ~mask_3) == size_3 && s.size() >= 2){
		len = 3;
		codepoint |= (buf[0] & mask_3) << 12;
		codepoint |= (buf[1] & mask_x) << 6;
		codepoint |= (buf[2] & mask_x) << 0;
	}
	else if ((first & ~mask_4) == size_4 && s.size() >= 2){
		len = 4;
		codepoint |= (buf[0] & mask_4) << 18;
		codepoint |= (buf[1] & mask_x) << 12;
		codepoint |= (buf[2] & mask_x) << 6;
		codepoint |= (buf[3] & mask_x) << 0;
	}
	else {
		return decode_error;
	}

	// Validation
	if(codepoint >= surrogate_range[0] && codepoint <= surrogate_range[1]){
		return decode_error;
	}
	if(len > 1 && (buf[1] < continuation_range[0] || buf[1] > continuation_range[1])){
		return decode_error;
	}
	if(len > 2 && (buf[2] < continuation_range[0] || buf[2] > continuation_range[1])){
		return decode_error;
	}
	if(len > 3 && (buf[3] < continuation_range[0] || buf[3] > continuation_range[1])){
		return decode_error;
	}

	return {codepoint, len};
}

struct Iterator {
	Slice<byte> data;
	isize current = 0;

	pair<rune, i32> next(){
        if(done()){
			return {0, 0};
		}
		auto res = utf8::decode(data.sub(current, data.size()));
		current += res.b;

		return res;
	}

	bool done() const {
		return current >= data.size();
	}
};
}/* namespace utf8 */

namespace cpp {
struct UTF8IteratorWrapper : public utf8::Iterator {
void operator++(){}

auto operator*(){
	return next();
}

bool operator!=(UTF8IteratorWrapper rhs){
	return current != rhs.current;
}
};
} /* namespace cpp */

struct string {
private:
	byte* data = nullptr;
	isize length = 0;

public:
	// Implicit constructor
	string(cstring cs){
		data = (byte *)(cs);
		length = cstring_len(cs);
	}

	string(){}

	static string from_bytes(Slice<byte> bytes){
		string s;
		s.data = bytes.raw_data();
		s.length = bytes.size();
		return s;
	}

	static string from_cstring(cstring cstr){
		return from_cstring(cstr, cstring_len(cstr));
	}

	static string from_cstring(cstring cstr, isize length){
		string s;
		s.data = (byte*)(cstr);
		s.length = length;
		return s;
	}

	isize size() { return length; }

	byte* raw_data() { return data; }

	bool operator==(string b){
		if(length != b.length){ return false; }

		for(isize i = 0; i < length; i += 1){
			if(data[i] != b.data[i]){ return false; }
		}

		return true;
	}

	utf8::Iterator iter(){
		utf8::Iterator it;
		it.data = Slice<byte>::from((byte*)(data), length);
		return it;
	}

	auto begin() const {
		cpp::UTF8IteratorWrapper it;
		it.data = Slice<byte>::from((byte*)(data), length);
		return it;
	}

	auto end() const {
		cpp::UTF8IteratorWrapper it;
		it.current = length;
		return it;
	}
};

struct StringBuilder {
	DynamicArray<byte> data;

	void push_rune(rune r){
		auto [bytes, n] = utf8::encode(r);
		data.append_slice(bytes.sub(0, n));
	}

	void push_string(string s){
		data.append_slice(Slice<byte>::from(s.raw_data(), s.size()));
	}
	
	void push_integer(i64 value, i8 base){
		if(value < 0){
			push_rune('-');
		}
		
		isize begin = data.size();
		isize end = 0;
		
		switch(base){
			case 2: {
				i64 n = abs(value);
				while(n > 0){
					auto val = bool(n & 1);
					n = n >> 1;
					push_rune(i32('0') + val);
				}
				end = data.size();
			} break;
			
			case 8: {
				i64 n = abs(value);
				while(n > 0){
					i32 val = n & 7;
					n = n >> 3;
					push_rune(i32('0') + val);
				}
				end = data.size();
			} break;
			
			case 10: {
				i64 n = abs(value);
				while(n > 0){
					i32 val = n % 10;
					n = n / 10;
					push_rune(i32('0') + val);
				}
				end = data.size();
			} break;
			
			case 16: { // TODO: Handle uppercase
				i64 n = abs(value);
				while(n > 0){
					i32 val = n & 15;
					n = n >> 4;
					if(val < 10){
						push_rune(i32('0') + val);
					}
					else {
						push_rune(i32('a') + val - 10);
					}
				}
				end = data.size();
			} break;
			
			default: {
				push_string("%!(UNKNOWN BASE)");
			} break;
		}

        auto digits = data.sub(begin, end);
        reverse(digits);
	}

	static auto create(Allocator allocator){
		StringBuilder sb;
		sb.data = DynamicArray<byte>::create(allocator);
		return sb;
	}
	
	void clear(){
		data.clear();
	}

	string build(){
		data.resize(data.size());
		auto s = data.extract_slice();
		return string::from_bytes(s);
	}

	auto allocator() const {
		return data.allocator;
	}
	
	void dealloc(){
		data.dealloc();
	}
};

void destroy(StringBuilder sb){
	sb.dealloc();
}
