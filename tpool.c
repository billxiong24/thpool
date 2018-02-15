#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "tpool.h"

#define MAX_THREADS 50
static int shutdown = 0;

struct job_thread {
    pthread_t thread;
    int id;
    TPOOL *tpool;
};

struct tpool_job {
    tpool_func func;
    call_back cb;
    void *arg;
};

struct TPOOL {
    //global lock for pool,, acquire when operating on struct elements
    pthread_mutex_t tpool_lock;
    pthread_cond_t tpool_signal;
    QUEUE *job_queue;
    struct job_thread *threads;

    size_t threads_started;
    bool shutdown;
};

static void *thread_run(void *arg) {
    struct job_thread *jobthread = arg;
    TPOOL *pool = jobthread->tpool;

    pthread_mutex_lock(&(pool->tpool_lock));
    pool->threads_started++;
    pthread_mutex_unlock(&(pool->tpool_lock));
    
    while(1) {
        pthread_mutex_lock(&(pool->tpool_lock));
        int shut = 0;
        while(queue_empty(pool->job_queue)) {
            //wait until a job becomes available
            pthread_cond_wait(&(pool->tpool_signal), &(pool->tpool_lock));
            if(shutdown == 1) {
                shut = 1;
                break;
            }
        }
        if(shut)
            break;
        

        struct tpool_job *job = queue_pop(pool->job_queue);

        pthread_mutex_unlock(&(pool->tpool_lock));

        //execute the job
        void *res = job->func(job->arg);
        //for now, no error
        job->cb(0, res);
    }

    //at this point, the thread still has the lock
    pthread_mutex_unlock(&(pool->tpool_lock));
    /*pool->threads_started--;*/
    pthread_exit(NULL);

    /*return NULL;*/
}

static void thread_init(TPOOL *pool, int i) {
    struct job_thread jobthread = pool->threads[i];
    jobthread.tpool = pool;
    jobthread.id = i;

    pthread_create(&jobthread.thread, NULL, thread_run, &jobthread);
    /*void *ret = NULL;*/
    //main thread does not wait for the compleition of this thread
    pthread_detach(jobthread.thread);
}

TPOOL *tpool_init(size_t num_threads) {
    size_t tpool_size = sizeof(TPOOL);
    TPOOL *pool = malloc(tpool_size);
    //zero out allocated memory, just in case 
    memset(pool, 0, tpool_size);
    //doesnt matter since memory was zeroed out, but for clarity
    pool->threads_started = 0;
    
    pthread_mutex_init(&(pool->tpool_lock), NULL);
    pthread_cond_init(&(pool->tpool_signal), NULL);

    pool->job_queue = queue_init();
    size_t size_els = sizeof(struct job_thread) * num_threads;
    pool->threads = malloc(size_els);

    memset(pool->threads, 0, size_els);
    pool->threads_started = 0L;

    for (int i = 0; i < num_threads; i++) {
        thread_init(pool, i);
    }

    //wait for all threads to start before returning pool to user
    while(pool->threads_started != num_threads);
    return pool;
}

void tpool_add_job(TPOOL *pool, tpool_func func, void *arg, call_back cb) {
    pthread_mutex_t pool_lock = pool->tpool_lock;

    //create new job
    struct tpool_job *job = malloc(sizeof(*job));
    job->func = func;
    job->arg = arg;
    job->cb = cb;

    //add to queue, whose operations are synchronized, no need to provide lock
    queue_push(pool->job_queue, job);

    //signal to wake up a sleeping thread
    pthread_cond_broadcast(&(pool->tpool_signal));
}

void tpool_free(TPOOL *pool){
    pthread_exit(NULL);

}
