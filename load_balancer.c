// Copyright Stan Andreea-Cristina 313CA
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "load_balancer.h"
#include "Hashtable.h"
#include "LinkedList.h"
#include "server.h"
#include "utils.h"

#define LABEL 100000

struct load_balancer {
	server_memory* server[LABEL];
	int size_server;
	unsigned int* hashring;
	int size_hashring;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}


void shift_function_right(int size, unsigned int hashring[],
							int value, int pos) {
    for ( int j = size; j > pos; j-- ) {
        hashring[j] = hashring[j-1];
    }
    hashring[pos] = value;
}

void shift_function_left(int size, unsigned int hashring[], int pos) {
    for ( int j = pos; j < size; j++ ) {
        hashring[j] = hashring[j+1];
    }
}

void shift_function_left_servers(int size, server_memory* server[LABEL],
								int pos) {
    for ( int j = pos; j < size; j++ ) {
        server[j] = server[j+1];
    }
}

/*
Functie care initializeaza structura load_balancer reprezentata prin pointerul
main, si aloca memorie pentru vectorul de hashring.
*/
load_balancer* init_load_balancer() {
    load_balancer *main;

    main = malloc(sizeof(load_balancer));
    DIE(main == NULL, "malloc failed");

	main->size_server = 0;
	main->hashring = (unsigned int *)calloc(3 * LABEL, sizeof(unsigned int));
	main->size_hashring = 0;

    return main;
}

/*
Functie care stocheaza o structura de tip cheie-valoare in serverul
(hashtable-ul) potrivit din vectorul de servere(hashtable-uri).
Dupa stocare, se modifica pointerul server_id in functie de serverul pe care a
fost stocata perechea cheie-valoare. 
In cazul in care hash-ul cheii este mai mare decat hash-ul tuturor serverelor
din hashring, adaugarea se va face in primul server din hashring.
*/
void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	for(int i = 0; i < main->size_hashring; i++) {
		if(hash_function_key(key) < hash_function_servers(&main->hashring[i])) {
			int id = main->hashring[i] % LABEL;

			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					server_store(main->server[j], key, value);

					*server_id = id;
					break;
				}
			}
			break;
		}
		if(hash_function_key(key) > hash_function_servers(&main->hashring[i]) &&
			i == main->size_hashring - 1) {
			int id = main->hashring[0] % LABEL;

			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					server_store(main->server[j], key, value);

					*server_id = id;
					break;
				}
			}
			break;
		}
	}
}

/*
Functie care intoarce valoarea asociata cheii trimise ca parametru functiei,
dupa ce se parcurge hashringul si se compara hash-ul corespunzator
pozitiei din hashring si cheie. In cazul in care hash-ul cheii este mai mare
decat hash-ul tuturor serverelor din hashring, valoarea se va cauta in primul
server din hashring.
*/
char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	for(int i = 0; i < main->size_hashring; i++) {
		if(hash_function_key(key) < hash_function_servers(&main->hashring[i])) {
			int id = main->hashring[i] % LABEL;

			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					*server_id = id;

					return server_retrieve(main->server[j], key);
				}
			}
			break;
		}
		if(hash_function_key(key) > hash_function_servers(&main->hashring[i]) &&
			i == main->size_hashring - 1) {
			int id = main->hashring[0] % LABEL;

			*server_id = id;

			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					return server_retrieve(main->server[j], key);
				}
			}
		}
	}
	return NULL;
}

/*
Functie care restabileste serverul in care este stocata perechea cheie-valoare,
comparand hash-ul fiecarui server din hashring si hash-ul cheii, avand grija ca
server-ul pe care se face redistribuirea sa difere de server-ul pe care a fost
stocata perechea cheie-valoare anterior.
In cazul in care hash-ul cheii este mai mare decat hash-ul tuturor serverelor
din hashring, adaugarea se va face in primul server din hashring, diferit de 
serverul initial.
*/
void loader_restore(load_balancer* main, char* key, char* value,
					int server_id) {
	for(int i = 0; i < main->size_hashring; i++) {
		int id = main->hashring[i] % LABEL;
		unsigned int hash_serv = hash_function_servers(&main->hashring[i]);
		int size = main->size_hashring;

		if(hash_function_key(key) < hash_serv && id != server_id) {
			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					server_store(main->server[j], key, value);

					break;
				}
			}
			break;
		}
		if(hash_function_key(key) > hash_serv && i == size - 1 && id != server_id) {
			id = main->hashring[0] % LABEL;

			for(int j = 0; j < main->size_server; j++) {
				if(main->server[j]->id_server == id) {
					server_store(main->server[j], key, value);

					break;
				}
			}

			break;
		}
	}
}

/*
Functie care intoarce vecinul din dreapta al serverului din hashring trimis ca
parametru functiei.
*/
int get_neighbor(load_balancer *main, int hashring_pos) {
	int neighbor;
	unsigned int check = main->hashring[hashring_pos] % LABEL;

	if(hashring_pos < main->size_hashring - 1) {
		if(main->hashring[hashring_pos+1] % LABEL == check) {
			neighbor = main->hashring[hashring_pos+2] % LABEL;
		}

		if(main->hashring[hashring_pos+2] % LABEL == check){
			neighbor = main->hashring[hashring_pos+3] % LABEL;
		}

		if(main->hashring[hashring_pos+1] % LABEL != check) {
			neighbor = main->hashring[hashring_pos+1] % LABEL;
		}
	} else if (hashring_pos == main->size_hashring - 1){
		neighbor = main->hashring[0] % LABEL;
	} else {
		if(main->hashring[0] % LABEL == check) {
			neighbor = main->hashring[1] % LABEL;
		}

		if(main->hashring[1] % LABEL == check){
			neighbor = main->hashring[2] % LABEL;
		}

		if(main->hashring[0] % LABEL != check) {
			neighbor = main->hashring[0] % LABEL;
		}
	}
	return neighbor;
}

/*
Functie care rebalanseaza obiectele stocate in servere. Acest lucru se
realizeaza dupa fiecare adaugare de server, comparand hash-ul cheilor stocate
in serverul(hashtable-ul)vecinului din dreapta al serverului adaugat, cu
hash-ul serverului trimis ca parametru functiei. 
Daca un server este adaugat la inceput in hashring, se iau si perechile
cheie-valoare din serverul vecinului, ce au hash-ul mai mare decat hash-ul
serverului adaugat. 
*/
void check_rebalance(load_balancer* main, int server_id) {
	for(int i = 0; i < 3; i++) {
		unsigned int label = i * LABEL + server_id;
		unsigned int hash_label = hash_function_servers(&label);

		for(int j = 0; j < main->size_hashring; j++) {
			if(main->hashring[j] == label) {
				int neighbor = get_neighbor(main, j);

				for(int k = 0; k < main->size_server; k++) {
					if(main->server[k]->id_server == neighbor) {
						for(unsigned int l = 0; l < main->server[k]->hmax; l++) {
							ll_node_t* current = main->server[k]->buckets[l]->head;

							while(current != NULL) {
								ll_node_t* next_node = current->next;
								char *key_cp = ((struct info*)current->data)->key;
								char *value_cp = ((struct info*)current->data)->value;
								int size, hashring_pos = j;

								if(hashring_pos > 0) {
									size = hashring_pos - 1;
									unsigned int hash_serv = hash_function_servers(&main->hashring[size]);

									if(hash_function_key(key_cp) < hash_label) {
										if(hash_function_key(key_cp) > hash_serv) {
											loader_restore(main, key_cp, value_cp, neighbor);

											server_remove(main->server[k], key_cp);
										}
									}
								}

								if(hashring_pos == 0) {
									size = main->size_hashring - 1;
									unsigned int hash_serv = hash_function_servers(&main->hashring[size]);

									if (hash_function_key(key_cp) > hash_label) {
										if(hash_function_key(key_cp) > hash_serv) {
											loader_restore(main, key_cp, value_cp, neighbor);

											server_remove(main->server[k], key_cp);
										}
									} else if (hash_function_key(key_cp) < hash_label) {
										loader_restore(main, key_cp, value_cp, neighbor);

										server_remove(main->server[k], key_cp);
									}
								}
								current = next_node;
							}
						}
						break;
					}
				}
				break;
			}
		}
	}
}

/*
Functie care adauga un nou server. Initial, se adauga structura de server in
vectorul destinat serverelor, apoi se calculeaza fiecare eticheta a serverului
adaugat si este adaugata in vectorul de hashring, la pozitia corecta in functie
de hash.
*/
void loader_add_server(load_balancer* main, int server_id) {
	server_memory *server = init_server_memory();
	int pos = main->size_server;

	main->server[pos] = server;
	main->server[pos]->id_server = server_id;
	main->size_server++;

	for(int i = 0; i < 3; i++) {
		int label = i * LABEL + server_id;
		unsigned int hash_label = hash_function_servers(&label);
		for(int j = 0; j < main->size_hashring; j++) {
			if(hash_function_servers(&main->hashring[j]) > hash_label) {
				shift_function_right(main->size_hashring, main->hashring, label, j);

				main->size_hashring++;
				break;
			}
		}
		int size = main->size_hashring;

		if(main->size_hashring == 0 ||
			hash_function_servers(&main->hashring[size - 1]) < hash_label) {
			main->hashring[size] = label;

			main->size_hashring++;
		}
	}

	check_rebalance(main, server_id);
}

/*
Functie care elimina un server si restabileste perechile cheie-valoare
stocate in acesta. Initial, se elimina etichetele replicilor din hashring,
apoi este parcurs hashtable-ul trimis ca parametru functiei si se restabileste
serverul in care sunt stocate perechile cheie-valoare.
*/
void loader_remove_server(load_balancer* main, int server_id) {
	for(int i = 0; i < 3; i++) {
		int label = i * LABEL + server_id;
		unsigned int hash_label = hash_function_servers(&label);

		for(int j = 0; j < main->size_hashring; j++) {
			if(hash_function_servers(&main->hashring[j]) == hash_label) {
				shift_function_left(main->size_hashring, main->hashring, j);

				main->size_hashring--;
				break;
			}
		}
		int size = main->size_hashring;

		if(hash_function_servers(&main->hashring[size - 1]) == hash_label) {
			main->size_hashring--;
		}
	}

	for(int i = 0; i < main->size_server; i++) {
		if(main->server[i]->id_server == server_id) {
			for(unsigned int j = 0; j <  main->server[i]->hmax; j++) {
				ll_node_t* current = main->server[i]->buckets[j]->head;

				while(main->server[i]->buckets[j]->size != 0) {
					char *key_cpy = ((struct info*)current->data)->key;
					char *value_cpy = ((struct info*)current->data)->value;

					loader_restore(main, key_cpy, value_cpy, server_id);

					server_remove(main->server[i], key_cpy);
				}
			}
			ht_free(main->server[i]);

			shift_function_left_servers(main->size_server, main->server, i);
			main->size_server--;

			break;
		}
	}
}

/*
Functie care elibereaza memoria stocata de load_balancer, de vectorul de
servere si de vectorul de hashring.
*/
void free_load_balancer(load_balancer* main) {
	for(int i = 0; i < main->size_server; i++) {
		free_server_memory(main->server[i]);
	}

	free(main->hashring);
	free(main->server);
}
