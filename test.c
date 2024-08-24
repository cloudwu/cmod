#include <stdio.h>
#include <assert.h>
#include "cmod.h"

struct test {
	int (*foo)(int n);
	void (*ud)(struct test *self);
};

static int
inc(int n) {
	return n + 1;
}

static int
dec(int n) {
	return n - 1;
}

static void
ud(struct test *self) {
	struct cmod * C = cmod_instance(self);
	printf("ud = %p\n", C->userdata(C));
}

static void
define_mod(struct cmod *C) {
	struct test *test = C->open(C, "test", sizeof(*test));
	test->foo = inc;
	test->ud = ud;
	int n = C->close(test);
	assert(n == 0);
	printf("foo(%d) = %d\n", 0, test->foo(0));
}

static void
test_mod(struct cmod *C) {
	struct test * test = C->import(C, "test");
	printf("test = %p\n", test);
	assert(cmod_instance(C) == cmod_instance(test));
	printf("foo(%d) = %d\n", 0, test->foo(0));
}

static void
test_ud(struct cmod *C) {
	struct test * test = C->import(C, "test");
	test->ud(test);
}

int
main() {
	struct cmod *C = cmod_instance(NULL);
	assert(C == cmod_instance(C));
	define_mod(C);
	test_mod(C);
	test_ud(C);
	char buffer[4096];
	struct cmod *C2 = C->new_instance(buffer, sizeof(buffer), C);
	struct test *test = C2->open(C2, "test", sizeof(*test));
	test->foo = dec;
	test->ud = ud;
	C2->close(test);

	test_mod(C2);
	test_ud(C2);
	
	return 0;
}
