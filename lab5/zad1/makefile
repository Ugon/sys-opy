SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/dircounter.c -o dircounter

.PHONY: clean
clean:
	rm -f dircounter
