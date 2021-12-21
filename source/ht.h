#include <stdio.h>
#include <stdio.h>
#include <stdint.h>

#define HT_EMPTY_KEY -1

typedef enum 
{
    EMPTY = 0,
    OCCUPIED = 1
} ht_entry_status;

typedef struct 
{
    uint32_t key;
    int value;
    int next;
    ht_entry_status status; 
} ht_entry;

typedef uint32_t (*hash_func)(uint32_t key, uint32_t pow_value);
typedef uint32_t (*hash_func_secondary)(uint32_t key, int iter, uint32_t capacity);

typedef struct 
{
    uint32_t capacity_pow;
    uint32_t capacity;
    hash_func h_func;
    hash_func_secondary h_func_secondary;
    ht_entry** entries;
} ht;


ht* ht_create(hash_func h_func, hash_func_secondary h_func_secondary, uint32_t h_capacity_pow);
ht_entry* ht_get_entry_and_prev(ht* h_table, uint32_t h_key, ht_entry** h_prev, uint32_t* h_hash);
void ht_insert(ht* h_table, uint32_t h_key, int h_value);
void ht_print_state(ht* h_table);
