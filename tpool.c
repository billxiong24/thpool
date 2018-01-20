#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "tpool.h"

struct TPOOL {
    pthread_mutex_t tpool_lock;
    QUEUE *job_queue;
    pthread_t *threads;
    size_t threads_started;
};

typedef struct {
    tpool_func func;
    void *arg;
} tpool_job;

static void *thread_run(void *arg) {
    return NULL;
}

TPOOL *tpool_init(size_t num_threads){
    size_t tpool_size = sizeof(TPOOL);
    TPOOL *pool = malloc(tpool_size);
    //zero out allocated memory, just in case 
    memset(pool, 0, tpool_size);
    
    pthread_mutex_init(&(pool->tpool_lock), NULL);

    pool->job_queue = queue_init();
    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    pool->threads_started = 0L;


    for (int i = 0; i < num_threads; i++) {
        pthread_t thread = pool->threads[i];
        pthread_create(&thread, NULL, thread_run, pool);
        
    }

    return pool;
}

void tpool_add_job(TPOOL *pool, tpool_func func, void *arg){

}

void tpool_free(TPOOL *pool){

}
