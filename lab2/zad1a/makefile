SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -Wall

ODIR = obj
CDIR = src

_OBJECTS =    bubble.o           \
              generator.o

EXECUTABLES = bubble             \
              generator

OBJECTS =    $(patsubst %,$(ODIR)/%,$(_OBJECTS))

.DEFAULTGOAL: default
default: $(EXECUTABLES)

$(EXECUTABLES): mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(ODIR)/$@.o -o $@

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLES)
	rm -fr $(ODIR)