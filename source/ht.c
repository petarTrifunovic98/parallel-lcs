#include <stdio.h>
#include <stdlib.h>

typedef struct 
{
    void* key;
    void* value;
} ht_entry;

typedef struct 
{
    void* hash_func;
    ht_entry* entries;
} ht;


ht* ht_create(void* hash_func, int capacity) 
{
    ht* created_ht = (ht*)malloc(sizeof(ht));
    created_ht->hash_func = hash_func;
    created_ht->entries = (ht_entry*)malloc(1.2 * capacity * sizeof(ht_entry));
}


