/**
 * This was a simple integer-integer hashmap I wrote for
 * my ATCS Compilers and Interpreters class. It can only
 * handle putting in and retrieving numbers, and can not
 * remove or search for elements in the map. It also tries
 * to simulate some level of object-oriented-ness (kind of lol).
 * 
 * For the purposes of the battleship project, the
 * NULL value placeholder has just been changed to -1
 * because it's easier and battleship never puts -1
 * in the map anyway.
 */

#include "./headers/hashmap.h"

int size = 1024; // size of 1 level of hashmap

/**
 * Knuth hash function
 * @param a the integer number
 */ 
int hash(int a) {         
    return (a*2654435761) % size;
}

// pseudo-linked list 
struct entry
{
    int key;          // unhashed int key
    int val;          // value
    struct entry *next; // next in the list
};

/**
 * Allocates space for the map and initializes -1 values
 */
struct entry *initializeHashmap()
{
    struct entry *map; // entry array

    map = malloc(size * sizeof(struct entry));

    for (int i = 0; i < size; i++)
    {
        map[i].key = -1;;
        map[i].next = NULL;
    }

    return map;
}

/**
 * Adds an entry to the map.
 * Case 1: the address is unoccupied in the array
 * Case 2: the address is occupied and the key exists
 * Case 3: the address is occupied but the key does not exist
 * 
 * @param ent pointer to the entry to add
 */
void addEntry(struct entry *ent, struct entry *map)
{
    int address = hash(ent->key);
    struct entry *current = map + address;

    // empty list, add key-value pair directly
    if (current->key == -1) map[address] = *ent;
    else
    {
        // get to the tail of the list or the first matching key
        while (current->next != NULL && ent->key != current->key) current = current->next;

        // key exists, just replace the value
        if (ent->key == current->key) current->val = ent->val;
        else current->next = ent; // otherwise concat it to the list
    }

    return;
}

int get(int key, struct entry *map)
{
    int address = hash(key);
    struct entry *current = map + address;

    int comparison = key - current->key;

    while (current->next != NULL && comparison != 0)
    {
        current = current->next;
        comparison = key - current->key;
    }

    return comparison == 0 ? current->val : -1;
}

/**
 * Puts a new value into the map
 * 
 * @param key the key
 * @param value the value
 * @param map the map
 */
void put(int key, int value, struct entry *map)
{
    struct entry e;

    e.key = key;
    e.val = value;
    e.next = NULL;

    addEntry(&e, map);

    return;
}