SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/wrfifo.c -o wrfifo
	$(CC) $(CFLAGS) $(CDIR)/rdfifo.c -o rdfifo

.PHONY: clean
clean:
	rm -f rdfifo wrfifo

