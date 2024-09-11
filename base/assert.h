#pragma once

#include "prelude.h"
#include <stdio.h>
#include <stdlib.h>

static inline
void debug_assert(bool pred, cstring msg){
	#ifdef NDEBUG
		(void)pred; (void)msg;
	#else
	if(!pred){
		fprintf(stderr, "Failed assert: %s\n", msg);
		abort();
	}
	#endif
}

static inline
void panic_assert(bool pred, cstring msg){
	if(!pred){
		fprintf(stderr, "Failed assert: %s\n", msg);
		abort();
	}
}

noreturn
static inline
void panic(char* const msg){
	fprintf(stderr, "Panic: %s\n", msg);
	abort();
}

noreturn
static inline
void unimplemented(){
	fprintf(stderr, "Unimplemented\n");
	abort();
}
