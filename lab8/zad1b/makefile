SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/generator.c -o generator -lpthread
	$(CC) $(CFLAGS) $(CDIR)/finder.c -o finder -lpthread

.PHONY: clean
clean:
	rm -f finder generator