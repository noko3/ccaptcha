/* Hash table implementation
 * Part of the denko project
 * 
 * written by noko3
 * License: GPLv2, see COPYING
 */
#include "hashtable.h"

static uint16_t hash(const char * string, unsigned len) {
	uint16_t H = len;
	while (*string) {
		H ^= *string;
		H = ~H;
		H ^= (H >> (*string % 16));
		H = (H>>13)|(H<<3);
		H = ((H&0b0101010101010101) << 1) | ((H&0b1010101010101010) >> 1);
		string++;
	}
	return H;
}

int htable_init(struct hash_table * HT, unsigned power) {
	HT->power = power;
	HT->table = calloc(sizeof(list_node *), power);
	HT->keys = malloc(0);
	HT->nkeys = 0;
	
	return 0;
}

int htable_kill(struct hash_table * HT) {
	unsigned i;
	for (i = 0; i < HT->nkeys; i++) {
		free(HT->keys[i]);
	}
	free(HT->keys);
	
	for (i = 0; i < HT->power; i++) list_kill(HT->table[i]);
	free(HT->table);
	free(HT);
	
	return 0;
}

int htable_set(struct hash_table *HT, const char *key, void *_value, size_t sz){
	if (key == NULL) return 0;
	
	int L = strlen(key);
	void *value;
	
	if (sz) {
		value = calloc(sizeof(char), sz);
		memcpy(value, _value, sz);
	} else
		value = NULL;
	
	uint16_t H = hash(key, L) % HT->power;
	
	list_node * S = HT->table[H];
	while (S != NULL) {
		if (S->name != NULL && !strcmp((const char*)(S->name), key)) {
			if (S->value != NULL) free(S->value);
			if (value != NULL) {
				S->value = calloc(sizeof(char), sz+1);
				memcpy(S->value, value, sz);
			}
			break;
		}
		S=S->next;
	}
	if (S == NULL)
	{
		list_node * C = list_init();
		C->name = (void *)calloc(sizeof(char), L+1);
		strncpy(C->name, key, L);
		
		if (value != NULL) {
			C->value = (void *)calloc(sizeof(char), sz+1);
			memcpy(C->value, value, sz);
		}
		
		list_add(&(HT->table[H]), C);
		if (!(HT->nkeys % HT->power)) {
			HT->keys = realloc(HT->keys, sizeof(char*)*(HT->nkeys + HT->power));
		}
		HT->keys[HT->nkeys] = calloc(sizeof(char), L+1);
		strncpy(HT->keys[HT->nkeys++], key, L);
	}
	
	if (value != NULL) free(value);
		
	return 0;
}

int htable_remove(struct hash_table * HT, const char * key) {
	int L = strlen(key);
	
	uint16_t H = hash(key, L) % HT->power;
	
	list_node * S = HT->table[H];
	
	while (S != NULL) {
		if (S->name != NULL && !strcmp((const char*)(S->name), key)) {
			if (S->value != NULL) free(S->value);
			if (S->name  != NULL) free(S->name);
			S=S->next;
			break;
		}
		S=S->next;
	}
	
	return 0;
}

void * htable_find(struct hash_table * HT, const char * key) {
	uint16_t H = hash(key, strlen(key)) % HT->power;
	
	list_node * C = HT->table[H];
	while (C != NULL) {
		if (C->name!=NULL && !strcmp((const char*)C->name, key)) return C->value;
		C=C->next;
	}
	return NULL; //nothing found
}

void htable_foreach(struct hash_table * HT, void(*function)(list_node * elem)) {
	unsigned i;
	for(i = 0; i < HT->power; i++)
	{
		function(HT->table[i]);
	}
}
