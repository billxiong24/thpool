#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Define incomplete struct type here, and definition in
 * queue.c to encapsulate struct definition from user.
 */
typedef struct QUEUE QUEUE;

QUEUE *queue_init();
void queue_push(QUEUE *, void *val);
void *queue_pop(QUEUE *);
void *queue_peek(QUEUE *);
void *queue_peek_tail(QUEUE *);
size_t queue_size(QUEUE *);
bool queue_empty(QUEUE *);
void queue_free(QUEUE *);

#endif
