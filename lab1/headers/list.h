#ifndef LIST_H
#define LIST_H
#include <stdbool.h>

typedef struct _List List;
typedef void*(*MappingFunction)(const void*);
typedef bool(*Predicate)(const void*);
typedef void(*PrintingFunction)(const void*);
typedef int(*ComparingFunction)(const void*, const void*);

List *createList();

void destroyList(List *list);

void addElement(List *list, void *elem);

void removeElement(List *list, void *elem);

void setOrdering(List *list, ComparingFunction compare);

void removeOrdering(List *list);

void *find(List *list, Predicate predicate);

//removes ordering
void map(List *list, MappingFunction function);

void filter(List *list, Predicate predicate);

void printList(List *list, PrintingFunction printElement);

#endif