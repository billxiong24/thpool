#include <stdio.h>
#include "tpool.h"
void *func(void *arg) {
    puts("hello world");
    return NULL;

}
void cb(int err, void *res) {
    puts("calling back");
}

int main(void) {
    TPOOL *tpool = tpool_init(5);
    tpool_add_job(tpool, func, NULL, cb);
    tpool_add_job(tpool, func, NULL, cb);
    while(1);
    return 0;
}
