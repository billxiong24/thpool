#include <stdio.h>
#include "tpool.h"
#include <unistd.h>
#include <pthread.h>
int finished = 0;
void *func(void *arg) {
    puts("hello world");
    return NULL;

}
void cb(int err, void *res) {
    puts("calling back");
    finished++;
}

void *func2(void *arg) {
    int *d = arg;
    printf("*d = %d\n", *d);
    finished++;
}

void *run(void *arg) {
    TPOOL *tpool = arg;
    tpool_add_job(tpool, func, NULL, cb);
    tpool_add_job(tpool, func, NULL, cb);
    int a = 3;
    int *d = &a;
    tpool_add_job(tpool, func2, (void *) d, cb);
    tpool_free(tpool);
}

int main(void) {
    TPOOL *tpool = tpool_init(5);
    pthread_t thread;

    pthread_create(&thread, NULL, run, tpool);
    pthread_join(thread, NULL);
}
