/* Hash table header
 * Part of the denko project
 * 
 * written by noko3
 * License: GPLv2, see COPYING
 */
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

struct hash_table {
	list_node ** table;
	char ** keys;
	unsigned power;
	unsigned nkeys;
};

int htable_init(struct hash_table * HT, unsigned power);
int htable_kill(struct hash_table * HT);
int htable_set(struct hash_table *HT, const char *key, void *_value, size_t sz);
int htable_remove(struct hash_table * HT, const char * key);
void * htable_find(struct hash_table * HT, const char * key);
void htable_foreach(struct hash_table * HT, void(*function)(list_node * elem));

#endif
