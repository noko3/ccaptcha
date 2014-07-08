/* Linked list header
 * Part of the denko project
 * 
 * written by noko3
 * License: GPLv2, see COPYING
 */
#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdio.h>
extern int keys_total;

typedef struct node {
	size_t size;
	void * name;
	void * value;
	struct node * next;
} list_node;

list_node * list_init();
int list_free(list_node * ent);
int list_kill(list_node * list);
int list_add(list_node ** list, list_node * ent);
int list_remove_next(list_node * prev);
int list_remove(list_node * list, list_node * ent);

#endif
