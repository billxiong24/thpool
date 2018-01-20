#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct queue_node {
    struct queue_node *next;
    void *val;
};

//private QUEUE definition to encapsulate struct from outside user

struct QUEUE {
    //syncrhonize queue operations with lock
    pthread_mutex_t queue_lock;

    struct queue_node *head;
    struct queue_node *tail;
    
    //number of items in queue
    size_t size;
};

static struct queue_node *new_node(void *val) {
    struct queue_node *node = malloc(sizeof(*node));
    node->val = val;
    node->next = NULL;
    return node;
}

QUEUE *queue_init() {
    QUEUE *queue = malloc(sizeof(*queue));
    memset(queue, 0, sizeof(*queue));
    pthread_mutex_init(&(queue->queue_lock), NULL);
    queue->head = queue->tail = NULL;
    queue->size = 0;

    return queue;
}

void queue_push(QUEUE *queue, void *val){
    pthread_mutex_lock(&(queue->queue_lock));

    struct queue_node *newnode = new_node(val);
    if(!queue->head) {
        queue->head = newnode;
        queue->tail = queue->head;
    }
    else {
        queue->tail->next = newnode;
        queue->tail = queue->tail->next;
    }
    queue->size++;
    pthread_mutex_unlock(&(queue->queue_lock));
}

void *queue_pop(QUEUE *queue){
    pthread_mutex_lock(&(queue->queue_lock));
    if(!queue->head)
        return NULL;

    struct queue_node *to_free = queue->head;
    void *ret = to_free->val;

    queue->head = queue->head->next;
    free(to_free);

    queue->size--;
    pthread_mutex_unlock(&(queue->queue_lock));
    return ret;
}

size_t queue_size(QUEUE *queue){
    return queue->size;
}

bool queue_empty(QUEUE *queue){
    return queue->size == 0;
}

void queue_free(QUEUE *queue) {
    pthread_mutex_lock(&(queue->queue_lock));

    struct queue_node *trav = queue->head;
    while(trav) {
        struct queue_node *next = trav->next;
        free(trav);
        trav = next;
    }

    pthread_mutex_unlock(&(queue->queue_lock));
    pthread_mutex_destroy(&(queue->queue_lock));
    free(queue);
}
