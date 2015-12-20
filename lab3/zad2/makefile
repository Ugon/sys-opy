SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -Wall

ODIR = obj
CDIR = src

_OBJECTS = file_counter.o         

OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))
EXECUTABLE = file_counter

.DEFAULT_GOAL: default
default: $(EXECUTABLE)
	cat README

$(EXECUTABLE): mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
	rm -fr $(ODIR)