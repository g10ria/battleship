#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct entry;

struct entry* initializeHashmap();

void addEntry(struct entry *ent, struct entry *map);

int get(int key, struct entry *map);

void put(int key, int value, struct entry *map);

void printKeys(struct entry *map);

int getSize();