SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src
HDIR = headers

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/tunel.c -o tunel -I$(HDIR)
	$(CC) $(CFLAGS) $(CDIR)/traffic.c -o traffic -I$(HDIR)

.PHONY: clean
clean:
	rm -f buffer tunel traffic