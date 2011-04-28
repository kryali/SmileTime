#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct queue_node {
	void* data;
  struct queue_node * next;
};

struct queue {
  queue_node* head;
  queue_node* tail;
};

/*char* strstp(char * str, char * stp, int * size);
int list_length(list * head);
void list_add(list ** head, char * name, char * ip, char * port, char protocol);
list * list_find(list * head, char * name);
void list_destroy(list ** head);
void list_print(list * head);
//char* strstp(char * str, char * stp, int * size);
*/


#include "linkedlist.c"
#endif
