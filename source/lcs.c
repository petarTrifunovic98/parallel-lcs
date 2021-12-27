#include "lcs.h"

# define WORD_BITSIZE __CHAR_BIT__ * sizeof(uint32_t)

int max(int a, int b)
{
    return a > b ? a : b;
}

int findLcs(char* arrA, char* arrB, uint32_t lenArrA, uint32_t lenArrB, ht* h_table)
{
    if (lenArrA == 0 || lenArrB == 0)
        return 0;

    uint64_t key = ((lenArrA - 1) << WORD_BITSIZE) || (lenArrB - 1);
    ht_entry* entry = ht_lookup(h_table, key);
    if (entry != NULL)
        return entry->value;
    
    if(arrA[lenArrA - 1] == arrB[lenArrB - 1])
    {
        ht_insert(h_table, key, 1 + findLcs(arrA, arrB, lenArrA - 1, lenArrB - 1, h_table));
        return ht_lookup(h_table, key)->value;
    }
    else
    {
        ht_insert(h_table, key, max(findLcs(arrA, arrB, lenArrA - 1, lenArrB, h_table), findLcs(arrA, arrB, lenArrA, lenArrB - 1, h_table)));
        return ht_lookup(h_table, key)->value;
    }
}