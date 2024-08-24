#include "cmod.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_MODULE_NAME 64
#define DEFAULT_CMOD_SIZE 0x10000
#define SIZEFUNC (sizeof(void *))
#define MIN_CMOD_SIZE (sizeof(struct cmod_impl) + 3 * sizeof(struct cmod_module) + 64 * SIZEFUNC)

struct cmod_module {
	char name[MAX_MODULE_NAME];
	int func_n;
	int func_head;
};

struct cmod_impl {
	size_t sz;
	int module_n;
	int module_close;
	int func_n;
	void *ud;
	struct cmod_impl *self;
	struct cmod api;
	struct cmod_module m[1];
};

static inline struct cmod_impl *
getC(struct cmod *C) {
	char * ptr = (char *)C;
	ptr -= offsetof(struct cmod_impl, api);
	return (struct cmod_impl *)ptr;
}

static inline void **
func_table(struct cmod_impl *C) {
	char * ptr = (char *)C;
	ptr = ptr + C->sz - C->func_n * SIZEFUNC;
	return (void **)ptr;
}

static struct cmod_module *
find_module(struct cmod_impl *C, const char *name, int n) {
	int i;
	for (i=0; i < n ; i++) {
		struct cmod_module *M = &C->m[i];
		if (strncmp(name, M->name, MAX_MODULE_NAME) == 0)
			return M;
	}
	return NULL;
}

static size_t
free_space(struct cmod_impl *C) {
	size_t sz = sizeof(*C) + (C->module_n - 1) * sizeof(C->m[0]) + C->func_n * SIZEFUNC;
	assert(sz <= C->sz);
	return C->sz - sz;
}

static void *
cmod_open(struct cmod *C_, const char *name, size_t sz) {
	struct cmod_impl *C = getC(C_);
	assert(C->module_n == C->module_close);
	assert(find_module(C, name, C->module_n) == NULL);
	int n = sz / SIZEFUNC;
	assert(n * SIZEFUNC == sz);
	size_t space = free_space(C);
	size_t need_space = sizeof(struct cmod_module) + sz + SIZEFUNC;
	if (need_space > space)
		return NULL;
	struct cmod_module * M = &C->m[C->module_n];
	void ** func = func_table(C);
	func -= n;
	memset(func, 0, sz);
	*(func-1) = C;
	strncpy(M->name, name, MAX_MODULE_NAME);
	M->func_n = n;
	M->func_head = C->func_n + n;
	
	C->func_n += n + 1;
	C->module_n ++;
	return (void *)func;
}

static int
cmod_close(void *type) {
	struct cmod_impl ** ptr = (struct cmod_impl **)type;
	struct cmod_impl *C = *(ptr-1);
	assert(C->module_n == C->module_close + 1);
	struct cmod_module *M = &C->m[C->module_close];
	int n = M->func_n;
	void ** f = func_table(C);
	f += C->func_n - M->func_head;
	int i;
	for (i=0;i<M->func_n;i++) {
		if (f[i])
			--n;
	}
	if (n == 0) {
		++C->module_close;
	}
	return n;
}

static void *
cmod_import(struct cmod *C_, const char *name) {
	struct cmod_impl *C = getC(C_);
	struct cmod_module *M = find_module(C, name, C->module_close);
	if (M) {
		void ** f = func_table(C);
		return (void *)&f[C->func_n - M->func_head];		
	}
	return NULL;
}

static void *
cmod_userdata(void *C_) {
	struct cmod_impl *C = getC(C_);
	return C->ud;
}

static void
init_api(struct cmod *api, struct cmod *(*new_instance)(void *buffer, size_t sz, void *ud)) {
	api->import = cmod_import;
	api->open = cmod_open;
	api->close = cmod_close;
	api->userdata = cmod_userdata;
	api->new_instance = new_instance;
}

static struct cmod *
cmod_new_instance(void *buffer, size_t sz, void *ud) {
	if (sz < MIN_CMOD_SIZE)
		return NULL;
	struct cmod_impl * C = (struct cmod_impl *)buffer;
	sz = (sz + SIZEFUNC - 1) / SIZEFUNC * SIZEFUNC;
	memset(buffer, 0, sz);
	init_api(&C->api, cmod_new_instance);
	C->self = C;
	C->sz = sz;
	C->ud = ud;
	return &C->api;
}

struct cmod *
cmod_instance(void *mod) {
	if (mod == NULL) {
		static char buffer[DEFAULT_CMOD_SIZE];
		struct cmod_impl * C = (struct cmod_impl *)&buffer;
		if (C->sz == 0) {
			init_api(&C->api, cmod_new_instance);
			C->self = C;
			C->sz = DEFAULT_CMOD_SIZE;		
		}
		return &C->api;
	} else {
		struct cmod_impl **ptr = (struct cmod_impl **)mod;
		struct cmod_impl *C = *(ptr-1);
		return &C->api;
	}
}
