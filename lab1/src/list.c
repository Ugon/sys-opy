#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "list.h"
#include "lambda.h"
#include "person.h"

typedef struct _Node Node;
struct _Node {
	void *elem;
	Node *next;
	Node *prev;
};

struct _List {
	Node *head;
	Node *tail;
	int (*ordering) (const void*, const void*);
	int   elemSize;
}; 

List *createList() {
	List *list = malloc(sizeof(List));
	
	list -> head     = NULL;
	list -> tail     = NULL;
	list -> ordering = NULL;

	return list;
}

void destroyList(List *list) {
	if (list == NULL) return;

	Node *h = list -> head;
	while (h != NULL) {
		Node *temp = h -> next;
		free(h);
		h = temp;
	}

	free(list);
}

//returns elem if elem is lesser than every other element in the list
Node *findSlot(List *list, void *elem) {

	if (list == NULL || elem == NULL) return NULL;
	if (list -> head == NULL || list -> ordering == NULL) return NULL;

	if((list -> ordering)(list -> head -> elem, elem) >= 0) return elem;
	
	Node *last = list -> head;
	while (last -> next != NULL && (list -> ordering)(last -> next -> elem, elem) < 0) last = last -> next;
	return last;
}

void attachBetween(Node *before, Node *after, Node *elem) {
	if (elem == NULL) return;

	if (before != NULL) before -> next = elem;
	if (after  != NULL) after  -> prev = elem;
	
	elem -> next = after;
	elem -> prev = before;
}

void detch(List *list, Node *elem){
	if (elem == NULL || list == NULL) return;
	if (list -> head == elem && list -> tail == elem) {
		list -> head = NULL;
		list -> tail = NULL;
	}
	else if (elem == list -> head) {
		list -> head = list -> head -> next;
		list -> head -> prev = NULL;
	}
	else if (elem == list -> tail) {
		list -> tail = list -> tail -> prev;
		list -> tail -> next = NULL;
	}
	else {
		elem -> prev -> next = elem -> next;
		elem -> next -> prev = elem -> prev;
	}

	elem -> next = NULL;
	elem -> prev = NULL;
}

void addElement(List *list, void *elem) {
	if (list == NULL || elem == NULL) return;

	Node *node = malloc(sizeof(Node));
	node -> elem = elem;

	if (list -> head == NULL) {
		list -> head = node;
		list -> tail = node;
	}
	else if (list -> ordering == NULL) {
		attachBetween(NULL, list -> head, node);
		list -> head = node;
	}
	else {
		Node *slot = findSlot(list, elem);
		if (slot == NULL) return;
		else if (slot == elem) {
			attachBetween(NULL, list -> head, node);
			list -> head = node;
		}
		else if (list -> tail == slot) {
			attachBetween(slot, NULL, node);
			list -> tail = node;
		}
		else {
			attachBetween(slot, slot -> next, node);
		}
	}
}

Node *findNode(List *list, void *elem) {
	if (list == NULL || elem == NULL) return NULL;
	Node *h = list -> head;
	while (h != NULL && h -> elem != elem) h = h -> next;
	return h;
}

void removeElement(List *list, void *elem) {
	if (list == NULL || elem == NULL) return;
	Node *n = findNode(list, elem);
	if (n != NULL) {
		detch(list, n);
		free(n);
	}
}

Node *largestBy(List *list, int(*compare)(const void*, const void*)) {
	if (list == NULL || compare == NULL) return NULL;
	if (list -> head == NULL) return NULL;

	Node* large = list -> head;
	Node* hd = list -> head -> next;

	while(hd != NULL) {
		if (compare(hd -> elem, large -> elem) > 0) large = hd;
		hd = hd -> next;
	}

	return large;
}

void setOrdering(List *list, int(*compare)(const void*, const void*)) {
    if (list == NULL || compare == NULL) return;

    Node *elem = largestBy(list, compare);
   	Node *newTail = elem;
   	Node *newHead = elem;
   	detch(list, elem);
   	attachBetween(NULL, NULL, elem);

   	while (list -> head != NULL) {
   		Node *elem = largestBy(list, compare);
   		detch(list, elem);
   		attachBetween(NULL, newHead, elem);
   		newHead = elem;
   	}

   	list -> head     = newHead;
   	list -> tail     = newTail;
    list -> ordering = compare;
}

void removeOrdering(List *list) {
	if (list == NULL) return;
	list -> ordering = NULL;
}

void map(List *list, void*(*function)(const void*)) {
	if (list == NULL || function == NULL) return;
	Node *h = list -> head;
	while (h != NULL) {
		h -> elem = function(h -> elem);
		h = h -> next;
	}
	list -> ordering = NULL;
}

void *find(List *list, bool(*predicate)(const void*)) {
	if (list == NULL || predicate == NULL) return NULL;
	Node *h = list -> head;
	while (h != NULL) {
		if (predicate(h -> elem)) return h -> elem;
		h = h -> next;
	}
	return NULL;
}

void filter(List *list, bool(*predicate)(const void*)) {
	if (list == NULL || predicate == NULL) return;
	Node *h = list -> head;
	while (h != NULL) {
		Node *temp = h -> next;
		if (!predicate(h -> elem)) detch(list, h);
		h = temp;
	}
}

void printList(List *list, void(*printElement)(const void*)) {
	if (list == NULL || printElement == NULL) return;
	printf("List{\n");
	Node *h = list -> head;
	while (h != NULL) {
		printElement(h -> elem);
		h = h -> next;
	}
	printf("}\n");
}
