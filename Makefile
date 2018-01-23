.DEFAULT_GOAL := main
CC=gcc
CFLAGS= -Wall
EXEC=main

LINKS=-lpthread

.PHONY: test clean 

$(EXEC): main.c queue.c tpool.c
	$(CC) -o $@ $^ $(LINKS)
