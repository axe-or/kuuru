#include "base.hpp"

namespace utf8 {

constexpr Array<rune, 2> surrogate_range = {0xd800, 0xdfff};
constexpr Array<byte, 2> continuation_range = {0x80, 0xbf};
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
pair<rune, i32> Iterator::next(){
	if(done()){
		return {0, 0};
	}
	auto res = utf8::decode(data.sub(current, data.size()));
	current += res.b;

	return res;
}

bool Iterator::done() const {
	return current >= data.size();
}

}/* namespace utf8 */