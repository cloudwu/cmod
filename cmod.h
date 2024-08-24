#ifndef CMOD_H
#define CMOD_H

#include <stddef.h>

struct cmod {
	void * (*import)(struct cmod *C, const char *name);
	void * (*open)(struct cmod *C, const char *name, size_t sz);
	int (*close)(void *type);
	void * (*userdata)(void *C);
	struct cmod *(*new_instance)(void *buffer, size_t sz, void *ud);
};

struct cmod * cmod_instance(void *);

#endif
