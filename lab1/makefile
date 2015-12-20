SHELL = /bin/sh

CC = gcc
CFLAGS = -std=c99 -I $(HDIR) -fPIC -Wall

HDIR = headers
ODIR = obj
CDIR = src
LDIR = libs


_HEADERS = lambda.h         \
           list.h           \
           person.h

_OBJECTS = list.o           \
           person.o         

HEADERS =    $(patsubst %,$(HDIR)/%,$(_HEADERS))
OBJECTS =    $(patsubst %,$(ODIR)/%,$(_OBJECTS))

.DEFAULT_GOAL: = help

$(EXECUTABLE): mk_dirs $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(ODIR)/%.o: $(CDIR)/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: help
help: 
	cat README

.PHONY: mk_dirs
mk_dirs:
	mkdir -p $(ODIR)
	mkdir -p $(LDIR)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
	rm -fr $(ODIR)

.PHONY: staticlib
staticlib: mk_dirs $(OBJECTS)
	ar crs $(LDIR)/liblist.a $(ODIR)/list.o
	ar crs $(LDIR)/libperson.a $(ODIR)/person.o
	gcc $(CFLAGS) -static -L./$(LDIR) $(CDIR)/test.c -o test -llist -lperson 
	./test

.PHONY: dynamiclib
dynamiclib: mk_dirs $(OBJECTS)
	gcc -shared $(CFLAGS) -o $(LDIR)/liblist.so $(ODIR)/list.o
	gcc -shared $(CFLAGS) -o $(LDIR)/libperson.so $(ODIR)/person.o
	gcc $(CFLAGS) -L./$(LDIR) -Wl,-rpath ./$(LDIR) -o test $(CDIR)/test.c -llist -lperson
	./test

.PHONY: dll
dll: mk_dirs $(OBJECTS)
	gcc -shared $(CFLAGS) -o $(LDIR)/liblist.so $(ODIR)/list.o
	gcc -shared $(CFLAGS) -o $(LDIR)/libperson.so $(ODIR)/person.o
	gcc $(CFLAGS) -Wl,-rpath ./$(LDIR) -DDLL -o test $(CDIR)/test.c -ldl
	./test