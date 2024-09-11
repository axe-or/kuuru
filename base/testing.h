#pragma once

#include "prelude.h"
#include <stdio.h>

#define RED(str_) ("\e[0;31m" str_ "\e[0m")
#define GREEN(str_) ("\e[0;32m" str_ "\e[0m")

struct Test {
	i32 total;
	i32 failed;
	cstring title;
};

static
void test_display_results(struct Test t){
	printf("[%s] %s ok in %d/%d\n",
		t.title,
		t.failed == 0 ? GREEN("PASS") : RED("FAIL"),
		t.total - t.failed,
		t.total
	);
}

static
bool test_expect(struct Test* t, bool predicate, i32 line, cstring file, cstring msg){
	t->total += 1;
	if(!predicate){
		printf("[Fail %s:%d] %s\n", file, line, msg);
		t->failed += 1;
	}
	return predicate;
}

#define Expect(expr_) test_expect(&_test, (expr_), __LINE__, __FILE__, #expr_)

#define Test_Begin(title_) struct Test _test = { .total = 0, .failed = 0, .title = (title_) }

#define Test_End() test_display_results(_test); return _test.failed == 0

#define Test_Func static inline bool

#undef RED
#undef GREEN
