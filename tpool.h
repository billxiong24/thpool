#ifndef TPOOL_H
#define TPOOL_H
#include <stddef.h>

typedef struct TPOOL TPOOL;
typedef void (*tpool_func)(void *arg);

TPOOL *tpool_init(size_t num_threads);
void tpool_add_job(TPOOL *pool, tpool_func func, void *arg);
void tpool_free(TPOOL *pool);

#endif
