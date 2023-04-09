// Copyright Stan Andreea-Cristina 313CA
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "LinkedList.h"
#include "server.h"

server_memory *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*));

void
ht_put(server_memory *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

void *
ht_get(server_memory *ht, void *key);

void
ht_remove_entry(server_memory *ht, void *key);

void
ht_free(server_memory *ht);

/*
 * Functii de comparare a cheilor:
 */
int
compare_function_ints(void *a, void *b);

int
compare_function_strings(void *a, void *b);

/*
 * Functii de hashing:
 */
unsigned int
hash_function_int(void *a);

unsigned int
hash_function_string(void *a);

#endif  // HASHTABLE_H_
