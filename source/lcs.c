#include "lcs.h"
#include <omp.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>

#define WORD_BITSIZE __CHAR_BIT__ * sizeof(uint32_t)
#define DOUBLE_WORD_BITSIZE __CHAR_BIT__ * sizeof(uint64_t)
int test;

int max(int a, int b)
{
    return a > b ? a : b;
}

int findLcs(char* arrA, char* arrB, uint32_t lenArrA, uint32_t lenArrB, ht* h_table)
{
    // if(__sync_val_compare_and_swap(&test, test, test) == 1)
    // {
    //     int threadNum = omp_get_thread_num();
    //     printf("%d: Exiting with lenA: %d, lenB: %d\n", threadNum, lenArrA, lenArrB);
    //     return 0;
    // }
    if (lenArrA == 0 || lenArrB == 0)
    {
        //printf("Executing for lengths %d and %d.\n", lenArrA, lenArrB);
        return 0;
    }

    uint64_t rightKeyPart = (uint64_t)(lenArrA);
    uint64_t leftKeyPart = ((uint64_t)(lenArrB)) << WORD_BITSIZE;
    uint64_t key = leftKeyPart | rightKeyPart;
    //printf("Executing for lengths %d and %d. Left key part is %ld. Right key part is %ld. Key is %ld.\n", lenArrA, lenArrB, leftKeyPart, rightKeyPart, key);
    ht_entry* entry = ht_lookup(h_table, key);
    if (entry != NULL) 
    {
        //printf("Entry found!\n");
        return entry->value;
    }
    
    if(arrA[lenArrA - 1] == arrB[lenArrB - 1])
    {
        int value = 1 + findLcs(arrA, arrB, lenArrA - 1, lenArrB - 1, h_table);
        ht_insert(h_table, key, value);
        return value;
    }
    else
    {
        int decider = rand();
        int value;
        if(decider % 2 == 0)
            value = max(findLcs(arrA, arrB, lenArrA - 1, lenArrB, h_table), findLcs(arrA, arrB, lenArrA, lenArrB - 1, h_table));
        else
            value = max(findLcs(arrA, arrB, lenArrA, lenArrB - 1, h_table), findLcs(arrA, arrB, lenArrA - 1, lenArrB, h_table));
        ht_insert(h_table, key, value);
        return value;
    }
}

uint32_t _primary_hash_function_fib(uint64_t h_key, uint32_t pow_value) //insert reference "hash func for int"
{
    const uint64_t fib_const = 11400714819323198485;
    return (h_key * fib_const) >> (DOUBLE_WORD_BITSIZE - pow_value);
}

uint32_t _secondary_hash_function(uint64_t h_key, int iter, uint64_t capacity)
{
    return (h_key + iter * iter) & (capacity - 1);
}

void main()
{
    srand(time(0));
    char* arrA = "asjwqepoizcnwrijljqwejlapzzcowqwejklaadqwekpzxcjklqnmqwejfkwpqomqopeorpakemngieoqweasdads \
        asdqweqwkjlqewjkqlwejkqweipocajlkasdnweqpoiqwekljadnasdjlkcznasdjlkqwekn";
    char* arrB = "sdfjklweropcnmcxvlksdpqksjapiasdnadsklqwenmzcxopqiwepoqiwmbjemvksqaoqwmewnqladqwejoqejwkj \
        asdjkqwehncxbmnefwjlkkgrbnfewlikewjkqkljwqenkfdsnmsdflkdsan,anmqewjlkqewkl;dsmncvxmnasdqweioqwjksanbmxzc";
    uint32_t lenArrA = 161;
    uint32_t lenArrB = 193;

    ht* table1 = ht_create(_primary_hash_function_fib, _secondary_hash_function, 16);
    ht* table = ht_create(_primary_hash_function_fib, _secondary_hash_function, 16);


    double start = omp_get_wtime();
    int lcsLen1 = findLcs(arrA, arrB, lenArrA, lenArrB, table1);
    double stop = omp_get_wtime();
    printf("Sequential execution time: %lf\n", stop-start);
    printf("\nLCS length: %d\n", lcsLen1);
    //printf("%d\n", omp_get_cancellation());
    test = 0;
    int numThreads;
    start = omp_get_wtime();
    #pragma omp parallel num_threads(6)
    {
        #pragma omp single
        {
            numThreads = omp_get_num_threads();
            for (int i = 0; i < numThreads; i++)
            {
                #pragma omp task
                {
                    int lcsLen = findLcs(arrA, arrB, lenArrA, lenArrB, table);
                    __sync_val_compare_and_swap(&test, 0, 1);
                    double currentStop = omp_get_wtime();
                    printf("\n%d: LCS length: %d calculated after %lf\n", omp_get_thread_num(), lcsLen, currentStop - start);
                    fflush(stdout);
                }
            }
        }
    }
    stop = omp_get_wtime();
    printf("Parallel execution time: %lf\n", stop-start);
}