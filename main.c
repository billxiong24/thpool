#include <stdio.h>
#include "queue.h"


int main(void) {

    QUEUE *queue = queue_init();

    queue_push(queue, "wot");
    queue_push(queue, "erer");
    queue_push(queue, "hi");
    queue_push(queue, "hi");
    queue_push(queue, "hi");
    queue_push(queue, "hi");
    puts(queue_pop(queue));
    puts(queue_pop(queue));
    /*while(!queue_empty(queue))*/
        /*puts(queue_pop(queue));*/


    queue_free(queue);
    
    
    return 0;
}
