/**
 * Simple hashmap in C. Only includes adding and fetching by key,
 * does not include removing key-value pairs.
 * 
 * Provide keys and values as strings. Pick a hash function
 * from ./hashfunctions.c and pick a map size in the variable size.
 */

#include "./headers/hashmap.h"

int size = 1024; // size of 1 level of hashmap

/**
 * Knuth hash function (integers only)
 * @param a the integer number
 * @param size the size of the map
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
 * Allocates space for the map and initializes null values
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

void printKeys(struct entry *map)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", map[i].key);
    }
    printf("\n");
}

int getSize()
{
    return size;
}

/**
 * Adds an entry to the map. Three cases: the address is unoccupied in the
 * array, the address is occupied and the key exists, and the address
 * is occupied but the key does not exist.
 * 
 * @param ent pointer to the entry to add
 */
void addEntry(struct entry *ent, struct entry *map)
{
    int address = hash(ent->key);
    struct entry *current = map + address;

    // empty list, add key-value pair directly
    if (current->key == -1) {
        map[address] = *ent;
    }
    else
    {
        // get to the tail of the list or the first matching key
        while (current->next != NULL && ent->key != current->key) {
            current = current->next;
        }

        // key exists, just replace the value
        if (ent->key == current->key) {
            current->val = ent->val;
        }
        else {
            current->next = ent; // otherwise concat it to the list
        }
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
 * Puts a new string value into the map given an integer key
 * 
 * @param key the key (an integer)
 * @param key the key (as a string)
 * @param value the value
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