# C Module

A simple C modules manager . You can put a bundle of function pointers into a module , and they can be used elsewhere at any time without linkers and loaders.

## Use module

A module is made up of several C functions, for example, we have a module named **foo**

```C
// in foo.h
// define the module foo as a C struct with function pointers
struct foo {
  int (*func)(void);
};
```

We can use this module via the module manager **cmod** .

```C
#include "cmod.h"
#include "foo.h"

int
foo(struct cmod *C) {
  // import module foo
  // NOTICE: The pointer mod never change, so you don't have to import it frequently.
  struct foo * mod = C->import(C, "foo");
  return foo->func();
}
```

The only export function in **cmod** is :
```C
struct cmod * cmod_instance(void *);
```

You can use `cmod_instance(NULL)` to get the global instance of **cmod**, and this instance can be obtained through any module pointers.

```C
struct foo * mod = C->import(C, "foo");
assert(cmod_instance(mod) == C);
assert(cmod_instance(C) == C);
```

**cmod** is also a module with a set of functions, I'll explain these APIs later.

```C
struct cmod {
	void * (*import)(struct cmod *C, const char *name);
	void * (*open)(struct cmod *C, const char *name, size_t sz);
	int (*close)(void *type);
	void * (*userdata)(void *C);
	struct cmod *(*new_instance)(void *buffer, size_t sz, void *ud);
};
```

## Define module

Use `cmod->open()` and `cmod->close()` to define a new module.

```C
#include "cmod.h"
#include "foo.h"

static int
foo_func(void) {
  return 42;
}

void
define_foo(struct cmod *C) {
  // sizeof(*foo) tells how many functions in module foo.
  struct foo *foo = C->open(C, "foo", sizeof(*foo));
  // You should assign every functions in module foo.
  foo->func = foo_func;
  int r = C->close(foo);
  // close returns the number of NULLs in module foo, it should be zero.
  assert(r == 0);
}
```

## Multiple module manager instance

The global cmod instance is very limited (64K RAM by default) . You can manage an unlimited number of instances by `cmod->new_instance()`.
The manager instance don't allocate the memory, you should designate a block of memory for it.

```C
struct cmod *
my_mod(struct cmod *C) {
  static char buffer[1024*1024];
  return struct cmod * inst = C->new_instance(buffer, sizeof(buffer), NULL);
}
```

## Userdata

Each module manager instance has an userdata (A pointer) , you can assign the userdata from beginning (`new_instance`) , and then you can't change it .
Use `C->userdata(C)` to get this userdata.
