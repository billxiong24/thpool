#ifndef TPOOL_H
#define TPOOL_H
#include <stddef.h>

typedef struct TPOOL TPOOL;
typedef void (*tpool_func)(void *arg);
typedef void (*call_back)(int err, void *res);

TPOOL *tpool_init(size_t num_threads);
void tpool_add_job(TPOOL *pool, tpool_func func, void *arg, call_back);
void tpool_free(TPOOL *pool);

#endif
