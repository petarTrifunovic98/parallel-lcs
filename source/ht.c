#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>

#include "ht.h"

#define WORD_BITSIZE __CHAR_BIT__ * sizeof(uint32_t)


ht* ht_create(hash_func h_func, hash_func_secondary h_func_secondary, uint32_t h_capacity_pow) 
{
    ht* created_ht = (ht*)malloc(sizeof(ht));
    created_ht->h_func = h_func;
    created_ht->h_func_secondary = h_func_secondary;
    uint32_t h_capacity = 1u << h_capacity_pow; //add overflow check
    created_ht->entries = (ht_entry**)malloc(/*1.2 **/ h_capacity * sizeof(ht_entry*));
    for(int i = 0; i < h_capacity; i++)
    {
        created_ht->entries[i] = (ht_entry*)malloc(sizeof(ht_entry));
        created_ht->entries[i]->status = EMPTY;
        created_ht->entries[i]->key = HT_EMPTY_KEY;
    }
    created_ht->capacity_pow = h_capacity_pow;
    created_ht->capacity = h_capacity;
}

ht_entry* ht_get_entry_and_prev(ht* h_table, uint32_t h_key, ht_entry** h_prev, uint32_t* h_hash)
{
    uint32_t hash = h_table->h_func(h_key, h_table->capacity_pow);
    uint32_t original_hash = hash;
    ht_entry* entry = h_table->entries[hash];
    *h_prev = NULL;
    int i = 1;


    while (entry->status != EMPTY && entry->key != h_key)
    {
        *h_prev = entry;
        hash = h_table->h_func_secondary(hash, i, h_table->capacity) & (h_table->capacity - 1); //capacity must be 2^n
        entry = h_table->entries[hash];
        i++;
        if (hash == original_hash) 
        {
            printf("Bad secondary hashing function!\n");
            return NULL;
        }
    }
    *h_hash = hash;
    return entry;
}

void ht_insert(ht* h_table, uint32_t h_key, int h_value)
{
    //add 'table full' check
    uint32_t hash;
    ht_entry** prev = (ht_entry**)malloc(sizeof(ht_entry*));
    ht_entry* entry = ht_get_entry_and_prev(h_table, h_key, prev, &hash);
    if (entry != NULL && entry->key == HT_EMPTY_KEY)
    {
        if(__sync_val_compare_and_swap(&entry->key, HT_EMPTY_KEY, h_key) != HT_EMPTY_KEY)
            return ht_insert(h_table, h_key, h_value);
        if(*prev != NULL)
        {
            (*prev)->next = hash;
        }
        entry->next = -1;
        entry->status = OCCUPIED;
        entry->value = h_value;
    }
    else if (entry == NULL) 
    {
        printf("Value %d with key %d could not be inserted due to bad secondary hash!\n", h_value, h_key);
    }
    else if (entry->key == h_key) 
    {
        printf("Value %d with key %d colud not be inserted because a value with the same key already exists!\n", h_value, h_key);
    }
}

void ht_print_state(ht* h_table)
{
    for (int i = 0; i < h_table->capacity; i++)
    {
        printf("%d: ", i);
        if (h_table->entries[i]->status == OCCUPIED)
            printf("key=%d, value=%d", h_table->entries[i]->key, h_table->entries[i]->value);
        printf("\n");
    }
    printf("\n");
}


/* -------Private methods-------- */
void _update_entry(ht_entry* entry, uint32_t h_key, int h_value, int next)
{
    entry->key = h_key;
    entry->value = h_value;
    entry->next = next;
}

uint32_t _primary_hash_function_fib(uint32_t h_key, uint32_t pow_value) //insert reference "hash func for int"
{
    const uint32_t fib_const = 2654435769;
    return (h_key * fib_const) >> (WORD_BITSIZE - pow_value);
}

uint32_t _secondary_hash_function(uint32_t h_key, int iter, uint32_t capacity)
{
    return (h_key + iter * iter) & (capacity - 1);
}

void main()
{
    ht* table = ht_create(_primary_hash_function_fib, _secondary_hash_function, 6);
    for (int i=0; i<64; i++)
    {
        ht_insert(table, rand() % 2048, rand() % 64);
        //printf("Inserted!\n");
    }
    ht_insert(table, 24u, 15);
    ht_print_state(table);
}