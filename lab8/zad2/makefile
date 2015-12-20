SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

CDIR = src

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/zad1.c -o zad1 -lpthread
	$(CC) $(CFLAGS) $(CDIR)/zad2.c -o zad2 -lpthread
	$(CC) $(CFLAGS) $(CDIR)/zad3.c -o zad3 -lpthread
	$(CC) $(CFLAGS) $(CDIR)/zad4.c -o zad4 -lpthread
	$(CC) $(CFLAGS) $(CDIR)/zad5.c -o zad5 -lpthread

.PHONY: clean
clean:
	rm -f zad1 zad2 zad3 zad4 zad5