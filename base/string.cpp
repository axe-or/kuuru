#include "base.hpp"

isize cstring_len(cstring cstr){
	constexpr isize MAX_CSTR_LEN = isize(~u32(0) >> u32(1));
	for(isize i = 0; i < MAX_CSTR_LEN; i += 1){
		if(cstr[i] == 0){
			return i;
		}
	}
	return MAX_CSTR_LEN;
}



// --- String ---
string string::from_bytes(Slice<byte> bytes){
	string s;
	s.data = bytes.raw_data();
	s.length = bytes.size();
	return s;
}

string string::from_cstring(cstring cstr){
	return from_cstring(cstr, cstring_len(cstr));
}

string string::from_cstring(cstring cstr, isize length){
	string s;
	s.data = (byte*)(cstr);
	s.length = length;
	return s;
}

utf8::Iterator string::iter(){
	utf8::Iterator it;
	it.data = Slice<byte>::from((byte*)(data), length);
	return it;
}

// --- String builder ---

void StringBuilder::push_rune(rune r){
	auto [bytes, n] = utf8::encode(r);
	data.append_slice(bytes.sub(0, n));
}

void StringBuilder::push_string(string s){
	data.append_slice(Slice<byte>::from(s.raw_data(), s.size()));
}

void StringBuilder::push_integer(i64 value, i8 base){
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

auto StringBuilder::create(Allocator allocator){
	StringBuilder sb;
	sb.data = DynamicArray<byte>::create(allocator);
	return sb;
}

string StringBuilder::build(){
	data.resize(data.size());
	auto s = data.extract_slice();
	return string::from_bytes(s);
}
