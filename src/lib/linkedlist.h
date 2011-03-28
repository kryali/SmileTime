#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node {
	char * name;
	char * ip;
    struct node * next;
};

typedef struct node list;

int list_length(list * head);
void list_add(list ** head, char * name, char * ip);
char * list_find(list * head, char * name);
void list_destroy(list ** head);
void list_print(list * head);


#include "linkedlist.c"
#endif