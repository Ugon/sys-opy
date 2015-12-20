SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -Wall

ODIR = obj
CDIR = src

_OBJECTS = dirs.o         

OBJECTS = $(patsubst %,$(ODIR)/%,$(_OBJECTS))
EXECUTABLE = dirs

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