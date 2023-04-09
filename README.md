# Load-Balancer
# Copyright Stan Andreea-Cristina 313CA

~load_balancer.c
    -shift_function_left/right/servers - Functii utilizate pentru a mentine
        crescator vectorul de hashring respectiv de servere, ordonate dupa
        hash.

    -loader_store - Intrucat in vectorul de hashring am stocat atat eticheta
        serverului cat si a replicilor sale, il parcurg si compar la fiecare
        pas hash-ul cheii trimise ca parametru si hash-ul etichetei, pentru
        a gasi serverul in care trebuie stocata perechea cheie-valoare.
        Odata gasit, parcurg vectorul de servere, pentru a gasi hashtable-ul
        potrivit id-ului, apoi adaug perechea in hashtable si modific valoarea
        poinetrului server_id trimis ca parametru functiei. 
        In cazul in care hash-ul cheii este mai mare decat hash-ul tuturor
        etichetelor din vector, adaug perechea cheie-valoare in primul
        hashtable de pe hashring.

    -loader_retrieve - Ca si la functia descrisa precedent, parcurg hashringul,
        compar hash-ul cheii cu cel al etichetelor, daca hash-ul cheii este mai
        mic, atunci apelez functia ce returneaza valoarea asociata cheii si
        modific valoarea pointerului server_id trimis ca parametru in functie.
        Tot ca mai devreme, tratez cazul particular in care hash-ul cheii este
        mai mare decat hash-ul tuturor etichetelor din hashring, caz in care
        caut cheia in primul server din vector, modificand si aici valoarea
        pointerului server_id.

    -loader_restore - Parcurg din nou vectorul de hashring, retin valoarea
        serverului aferent fiecarui label si, de data aceasta, verific atat
        ca hash-ul cheii sa fie mai mic decat cel al serverului, dar si ca
        distribuirea sa nu se faca pe acelasi server din care provenea cheia.
        Si in aceasta functie tratez cazul in care cheia este mai mare decat
        toate elementele din hashring, astfel ca redistribuirea se va face pe
        hashtable-ul primului server din hashring. In ambele cazuri, in final
        apelez functie de adaugare a perechii cheie-valoare in hashtable.

    -get_neighbor - Aceasta functie este folosita drept suport pentru functia
        urmatoare, in care rebalansez obiectele, pentru a-mi returna vecinul
        imediat urmator(din dreapta serverului adaugat in hashring), in functie
        de pozitia in care a fost adaugat noul server, avand grija sa iau exact
        valoarea serverului si nu a vreunei replici.

    -check_rebalance - Incep prin a calcula eticheta serverului
        adaugat, cat si a replicilor sale, apoi parcurg hashringul pana ajung
        la eticheta calculata, extrag vecinul din dreapta cu functia ajutatoare
        prezentata mai sus, apoi parcurg vectorul de servere pentru a ajunge la 
        hashtable-ul vecinului aflat anterior. 
        Parcurg hastable-ul si compar hash-ul fiecarei chei cu hash-ul
        etichetei. In cazul in care este mai mic, redistribui perechea
        cheie-valoare in noul hashtable si elimin intrarea din hashtable-ul
        curent. Si aici este un caz particular, cel in care serverul nou
        adaugat este pus pe prima pozitie in hashring, caz in care trebuie sa 
        caut si hashul cheilor care este mai mare decat hashul serverului nou
        adaugat si decat cel in care se afla initial, apoi procedez ca mai sus.

    -loader_add_server - La inceput initializez, hashtable-ul pentru noul
        server, apoi adaug structura acestuia in vectorul alocat pentru servere
        Pentru stocarea in hashring, calculez mai intai toate cele 3 etichete,
        hash-ul, apoi parcurg hashringul si caut primul hash din elementele
        acestuia mai mare decat hash-ul etichetei curente, si-l adaug direct
        la pozitia in care vectorul ramane sortat, shiftand vectorul la dreapta.
        Aici sunt doua cazuri particulare care se pot combina in aceleasi
        operatii, mai exact atunci cand vectorul e gol, si cel in care hash-ul
        etchetei este mai mare decat toate hash-urile deja existente, cazuri in
        care eticheta este adaugata la pozitia dimensiunii hashringului.
        In final se creste dimensiunea hashringului.

    -loader_remove_server - Initial elimin toate etichetele din hashring si
        shiftez la stanga toate elementele, apoi parcurg vectorul de servere
        pana ajung la hashtable-ul serverului cautat, parcurgand si
        redistribuind fiecare cheie-valoare din acesta, apoi eliminand intrarea
        din el. La finalul parcurgerii hashtable-ului, eliberez memoria alocata
        pentru acesta si shiftez la stanga vectorul de servere pentru a-l
        elimina din el. 

    free_load_balancer - Elibereaza memoria fiecarui hashtable din vectorul de
        servere, precum si a vectorilor de servere si de hashring.

~server.c - Toate functiile sunt folosite pentru a face legatura intre
    load_balancer si hashtable.

~Hashtable.c
    -ht_create - Initializeaza hashtable-ul si aloca memoria pentru fiecare
        lista din hashtable. De asemenea, creeaza pointeri la functiile de
        compare si hash folosite.
    
    -ht_put - Incep prin a calcula indexul la care voi stoca perechea
        cheie-valoare in hashtable, verific sa nu mai existe cheia in
        hashtable, iar daca nu este, pentru a evita cazul in care cheia poate
        fi modificata din exterior, aloc static o structura noua de tip 
        cheie-valoare si realizez deep copy, apoi adaug structura in lista.
        In cazul in care a fost gasita anterior cheia in hashtable, doar
        actualizez valoarea.

    -ht_get - Incep din nou prin a calcula indexul, de data asta de unde voi
        extrage valoarea ce se regaseste la cheia data ca parametru functiei.
        parcurg hashtable-ul pana gasesc cheia, apoi returnez valoarea sau NULL
        in cazul cheia nu exista.

    -ht_remove_entry - Calculez din nou indexul, apoi parcurg hashtable-ul pana
        gasesc pozitia la care se afla perechea cheie-valoare cautata, retinand
        intr-o variabila pozitia. Apelez functia de remove a pozitiei din lista
        apoi eliberez memoria nodului proaspat eliminat.

    -ht_free - Parcurg hashtable-ul si fiecare lista din acesta, eliminand
        initial toate intrarile de tip cheie-valoare din liste, apoi listele,
        si in final memoria folosita pentru a stoca hashtable-ul.

~Linked_List.c
    -ll_add_nth_node - Dupa ce am alocat memorie pentru noul nod din lista,
        am realizat deep copy pentru structura cheie-valoare ce va fi stocata,
        apoi, daca lista era goala sau nu, faceam legaturile necesare intre
        nodurile din lista.

    -ll_remove_nth_node - Dupa ce ajung la nodul din lista potrivit, elimin
        nodul de tip cheie-valoare. Functia intoarce un pointer spre acest nod
        proaspat eliminat din lista. Este responsabilitatea apelantului sa
	    elibereze memoria acestui nod. Dupa eliminarea nodului,
	    refac legaturile listei si scad marimea listei.

    -ll_free - Functia elibereaza memoria folosita de toate nodurile din lista,
	    iar la sfarsit, elibereaza memoria folosita de structura lista si
	    seteaza pointerul alocat listei pe NULL.
