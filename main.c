#include <stdio.h>
#include "tpool.h"
#include <unistd.h>
#include <pthread.h>
int finished = 0;
void *func(void *arg) {
    puts("executing job");
    sleep(1);
    return NULL;
}
void cb(int err, void *res) {
    puts("calling back");
    finished++;
}

void *run(void *arg) {
    TPOOL *tpool = arg;
    int count = 0;
    while(count++ < 1) {
        tpool_add_job(tpool, func, NULL, cb);
        sleep(2);
    }
}

int main(void) {
    TPOOL *tpool = tpool_init(2);
    pthread_t thread;

    pthread_create(&thread, NULL, run, tpool);
    pthread_join(thread, NULL);
    puts("heliro");
    tpool_free(tpool);
}
