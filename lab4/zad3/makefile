SHELL = /bin/sh

CC = gcc
CFLAGS = -std=gnu99 -Wall

ODIR = obj
CDIR = src

_OBJECTS = przodek.o         \
           potomek.o

OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))
EXECUTABLE = przodek         \
             potomek

.DEFAULT_GOAL: default
.PHONY: default
default:
	$(CC) $(CFLAGS) $(CDIR)/przodek.c -o przodek
	$(CC) $(CFLAGS) $(CDIR)/potomek.c -o potomek

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
	rm -fr $(ODIR)
