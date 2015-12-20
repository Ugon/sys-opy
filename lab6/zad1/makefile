SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src
HDIR = headers

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/client.c -o client -I$(HDIR)
	$(CC) $(CFLAGS) $(CDIR)/server.c -o server -I$(HDIR)

.PHONY: clean
clean:
	rm -f client server
