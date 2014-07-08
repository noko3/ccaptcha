/* Linked list implementation
 * Part of the denko project
 * 
 * written by noko3
 * License: GPLv2, see COPYING
 */
#include "list.h"

list_node * list_init() {
	list_node * n;
	n = malloc(sizeof(list_node));
	n->size = 0;
	n->name = NULL;
	n->value = NULL;
	n->next = NULL;
	return n;
}

int list_free(list_node * ent) {
	if(ent == NULL) return 0;
	if(ent->name != NULL) free(ent->name);
	if(ent->value != NULL) free(ent->value);
	free(ent);
	ent = NULL;
	return 0;
}

int list_kill(list_node * list) {
	if(list == NULL) return 0;
	list_node * n;
	while(list->next != NULL) {
		n = list;
		list = list->next;
		list_free(n);
	}
	list_free(list);
	return 0; //success
}

int list_add(list_node ** list, list_node * ent) {
	ent->next = *list;
	*list=ent;
	return 0; //success
}

int list_remove_next(list_node * prev) {
	if(prev->next == NULL) return 1; // cannot delete NULL
	list_node * n = prev->next;
	prev->next = n->next;
	list_free(n);
	return 0; //success
}
int list_remove(list_node * list, list_node * ent) {
	while(list->next != ent && list != NULL)
		list = list->next;
	if(list == NULL) return 2; // entity was not found in list
	return list_remove_next(list);
}
