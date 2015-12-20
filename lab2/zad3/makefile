SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -Wall

ODIR = obj
CDIR = src

_OBJECTS = locks.o         

OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))
EXECUTABLE = locks

$(EXECUTABLE): mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(ODIR)/%.o: $(CDIR)/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
	rm -fr $(ODIR)