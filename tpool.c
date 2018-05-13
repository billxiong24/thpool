#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "tpool.h"

struct job_thread {
    pthread_t thread;
    int id;
    bool is_executing;
    TPOOL *tpool;
};

struct tpool_job {
    tpool_func func;
    call_back cb;
    void *arg;
};

struct TPOOL {
    //global lock for pool, acquire when operating on struct elements
    pthread_mutex_t tpool_lock;
    pthread_cond_t tpool_signal;
    QUEUE *job_queue;

    QUEUE *job_threads;

    size_t threads_started;
    bool shutdown;
};

static void *thread_run(void *arg) {
    struct job_thread *jobthread = (struct job_thread *) arg;
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

            printf("pool->shutdown = %d\n", pool->shutdown);
            if(pool->shutdown) {
                shut = true;
                break;
            }
        }

        if(shut) {
            break;
        } 
        
        //at this point, broadcasted and queue not empty 
        struct tpool_job *job = queue_pop(pool->job_queue);
        jobthread->is_executing = true;

        pthread_mutex_unlock(&(pool->tpool_lock));

        //execute the job
        void *res = job->func(job->arg);
        //job is finished
        jobthread->is_executing = false;
        //for now, no error
        job->cb(0, res);
        free(job);
    }

    //at this point, the thread still has the lock
    pthread_mutex_unlock(&(pool->tpool_lock));
    puts("exiting");
    pool->threads_started--;
    pthread_exit(NULL);
    return NULL;
}

static void thread_init(TPOOL *pool, int i) {
    struct job_thread *tail = (struct job_thread *) queue_peek_tail(pool->job_threads);
    struct job_thread jobthread = *tail;
    jobthread.tpool = pool;
    jobthread.id = i;

    pthread_create(&jobthread.thread, NULL, thread_run, &jobthread);
    
    //main thread does not wait for the completion of this thread
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
    pool->job_threads = queue_init();

    pool->threads_started = 0L;

    for (int i = 0; i < num_threads; i++) {
        struct job_thread *jt = malloc(sizeof(*jt));
        memset(jt, 0, sizeof(jt));
        queue_push(pool->job_threads, jt);
        thread_init(pool, i);
    }

    //wait for all threads to start before returning pool to user
    while(pool->threads_started != num_threads) {
        continue;
    }
    
    return pool;
}

static bool check_available_threads(QUEUE *queue, void *arg) {
    struct job_thread *jt = (struct job_thread *) arg;
    /*printf("jt->is_executing = %d\n", jt->is_executing);*/
    return jt->is_executing;
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

    //TODO check if there is a thread that can handle this job. If not, add it dynamically.
    int job_diff = queue_size(pool->job_queue) - queue_size(pool->job_threads);

    //not enough threads
    if(job_diff > 0) {
        int old_thrd_started = pool->threads_started;
        struct job_thread *thread = queue_peek_tail(pool->job_threads);
        int last_id = thread->id;
        
        for(int i = 0; i < job_diff; i++) {
            struct job_thread *jt = malloc(sizeof(*jt));
            memset(jt, 0, sizeof(jt));
            queue_push(pool->job_threads, jt);
            //continue ids from last id, increment
            thread_init(pool, i + last_id);
        }

        //wait for all the new threads to be initialized
        while(pool->threads_started != old_thrd_started + job_diff) {
            continue;
        }
    }
    //signal to wake up a sleeping thread
    pthread_cond_broadcast(&(pool->tpool_signal));

    /*bool threads_available = queue_iter_cond(pool->job_threads, check_available_threads);*/
}

void tpool_free(TPOOL *pool){
    queue_free(pool->job_threads);
    queue_free(pool->job_queue);
    pthread_mutex_destroy(&(pool->tpool_lock));
    pool->shutdown = true;
    pthread_cond_broadcast(&(pool->tpool_signal));
    pthread_cond_destroy(&(pool->tpool_signal));
    free(pool);

    /*pthread_exit(NULL);*/
}
