#include "base.hpp"

#include <iostream> /* DEBUG ONLY. DON'T USE iostream, IT SUCKS. */

void assert(bool cond, cstring msg){
	if(!cond){
		std::fprintf(stderr, "Assert failed: %s\n", msg);
		throw "FAILED ASSERT";
	}
}

[[noreturn]]
void unimplemented(cstring msg){
	std::fprintf(stderr, "Unimplemented code: %s\n", msg);
	throw "UNIMPLEMENTED";
}

[[noreturn]]
void panic(cstring msg){
	std::fprintf(stderr, "Panic: %s\n", msg);
	throw "PANIC";
}

// template<typename T>
// void print(T x){
// 	std::cout << x << '\n';
// }

// template<typename T, typename ...Args>
// void print(T x, Args&& ...args){
// 	std::cout << x << ' ';
// 	print(args...);
// }

// template<typename A>
// void print_arr(A arr){
// 	for(isize i = 0; i < arr.size(); i++){
// 		std::cout << arr[i] << ' ';
// 	}
// 	std::cout << '\n';
// }

