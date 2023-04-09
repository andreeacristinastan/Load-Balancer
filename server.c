// Copyright Stan Andreea-Cristina 313CA
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "load_balancer.h"
#include "LinkedList.h"
#include "server.h"
#include "Hashtable.h"

#define HMAX 10000
/*
Functie de initializare a serverului, reprezentat de un hashtable.
*/
server_memory* init_server_memory() {
	server_memory* ht;

	ht = ht_create(HMAX, hash_function_key,
				compare_function_strings);

	return ht;
}

/*
Functie ce stocheaza o structura de tip cheie-valoare in hashtable.
*/
void server_store(server_memory* server, char* key, char* value) {
	ht_put(server, key, strlen(key) + 1, value, strlen(value) + 1);
}

/*
Functie care elimina intrarea din hashtable asociata cheii trimise ca parametru
*/
void server_remove(server_memory* server, char* key) {
	ht_remove_entry(server, key);
}

/*
Functie care returneaza valoarea asociata cheii trimise ca parametru.
*/
char* server_retrieve(server_memory* server, char* key) {
	return ht_get(server, key);
}

/*
Functie care elibereaza memoria server-ului.
*/
void free_server_memory(server_memory* server) {
	ht_free(server);
}
