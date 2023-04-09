// Copyright Stan Andreea-Cristina 313CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "server.h"
#include "Hashtable.h"

#define MAX_BUCKET_SIZE 64
#define RESIZETHRESHOLD 0.55
/*
Functii de comparare a cheilor:
*/
int
compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

server_memory *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	server_memory* ht;

	ht = malloc(sizeof(server_memory));
	DIE(ht == NULL, "malloc failed");

	ht->buckets = malloc(hmax * sizeof(linked_list_t*));
	DIE(ht->buckets == NULL, "malloc failed");

	for(unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(struct info));
	}

	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;

	return ht;
}

/*
Functie folosita pentru a stoca o structura de tip cheie-valoare in hashtable.
Functia creeaza cate o copie a valorilor la care pointeaza key si value,
adresele acestor copii sunt salvate in structura info asociata intrarii din ht.
*/
void
ht_put(server_memory *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	DIE(ht == NULL, "malloc failed");

	int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* curr = ht->buckets[index]->head;

	while(curr != NULL &&
			ht->compare_function(((struct info*)curr->data)->key, key) != 0) {
		curr = curr->next;
	}

	if(curr == NULL) {
		struct info new;

		new.key = malloc(key_size);
		DIE(new.key == NULL, "malloc failed");
		new.value = malloc(value_size);
		DIE(new.value == NULL, "malloc failed");

		memcpy(new.key, key, key_size);
		memcpy(new.value, value, value_size);

		ll_add_nth_node(ht->buckets[index], 0, &new);
		ht->size++;
		return;
	} else {
		memcpy(((struct info*)curr->data)->value, value, value_size);
		return;
	}
}

/*
Functie care returneaza valoarea din hashtable asociata cheii trimise ca
parametru in functie.
*/
void *
ht_get(server_memory *ht, void *key)
{
	DIE(ht == NULL, "malloc failed");

	int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* current = ht->buckets[index]->head;

	while(current != NULL &&
			ht->compare_function(((struct info*)current->data)->key, key) != 0) {
		current = current->next;
	}

	if( current != NULL ) {
		return ((struct info*)current->data)->value;
	}
	return NULL;
}

/*
Functie care elimina intrarea din hashtable asociata cheii trimise ca parametru
functiei. Dupa ce este returnat nodul corespunzator din lista, se elibereaza
memoria aferenta.
*/
void
ht_remove_entry(server_memory *ht, void *key)
{
	DIE(ht == NULL, "malloc failed");

	int index = ht->hash_function(key) % ht->hmax;
	int pos = 0;

	ll_node_t* current = ht->buckets[index]->head;

	while(current != NULL &&
			ht->compare_function(((struct info*)current->data)->key, key) != 0) {
		current = current->next;
		pos++;
	}

	if( current == NULL ) {
		return;
	}

	ll_node_t* removed = ll_remove_nth_node(ht->buckets[index], pos);

	free(((struct info*)removed->data)->key);
	free(((struct info*)removed->data)->value);
	free(removed->data);
	free(removed);
}

/*
Functie care elibereaza memoria folosita de toate intrarile din hashtable si
memoria folosita pentru a stoca hashtable-ul.
*/
void
ht_free(server_memory *ht)
{
	DIE(ht == NULL, "malloc failed");

	ll_node_t* current, *node;

	for(unsigned int i = 0; i < ht->hmax; i++) {
		current = ht->buckets[i]->head;

		while(current != NULL) {
			node = current;
			current = current->next;

			ll_node_t* removed = ll_remove_nth_node(ht->buckets[i], 0);

			free(((struct info*)node->data)->key);
			free(((struct info*)node->data)->value);
			free(removed->data);
			free(removed);
			ht->size--;
		}
		free(ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}
