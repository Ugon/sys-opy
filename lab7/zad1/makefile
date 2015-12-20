SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src
HDIR = headers

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/buffer.c -o buffer -I$(HDIR)
	$(CC) $(CFLAGS) $(CDIR)/producer.c -o producer -I$(HDIR)
	$(CC) $(CFLAGS) $(CDIR)/consumer.c -o consumer -I$(HDIR)

.PHONY: clean
clean:
	rm -f buffer producer consumer