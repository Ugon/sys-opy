SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -Wall

ODIR = obj
CDIR = src

_OBJECTS = processes.o         

OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))
EXECUTABLE = processes

.DEFAULT_GOAL: default
.PHONY: default
default:
	cat README

.PHONY: fork
fork: mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(CDIR)/processes.c -o $(EXECUTABLE) -DFORK

.PHONY: vfork
vfork: mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(CDIR)/processes.c -o $(EXECUTABLE) -DVFORK

.PHONY: clone
clone: mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(CDIR)/processes.c -o $(EXECUTABLE) -DCLONE

.PHONY: vclone
vclone: mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(CDIR)/processes.c -o $(EXECUTABLE) -DVCLONE

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
	rm -fr $(ODIR)
