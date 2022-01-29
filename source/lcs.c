#include "lcs.h"
#include <omp.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define WORD_BITSIZE __CHAR_BIT__ * sizeof(uint32_t)
#define DOUBLE_WORD_BITSIZE __CHAR_BIT__ * sizeof(uint64_t)
#define MAX_ARRAY_SIZE 1024
int test;

int max(int a, int b)
{
    return a > b ? a : b;
}

int findLcs(char* arrA, char* arrB, uint32_t lenArrA, uint32_t lenArrB, ht* h_table)
{
    #ifdef TEST_EXIT_CONDITION
    if(__sync_val_compare_and_swap(&test, test, test) == 1)
    {
        int threadNum = omp_get_thread_num();
        //printf("%d: Exiting with lenA: %d, lenB: %d\n", threadNum, lenArrA, lenArrB);
        return 0;
    }
    #endif

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
        //int decider = 2;
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

int _read_lcs_args_from_file(char* filename, uint32_t* htPow, uint32_t* lenArrA, uint32_t* lenArrB, char** arrA, char** arrB)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error openning file %s!\n", filename);
        return 1;
    }

    int intMaxChars = 4;
    char intBuff[intMaxChars];
    char strBuffA[MAX_ARRAY_SIZE];
    char strBuffB[MAX_ARRAY_SIZE];

    fgets(intBuff, intMaxChars, file);
    *htPow = atoi(intBuff);

    fgets(strBuffA, MAX_ARRAY_SIZE, file);
    *lenArrA = strlen(strBuffA);
    if(strBuffA[(*lenArrA) - 1] == '\n')
    {
        strBuffA[(*lenArrA) - 1] = '\0';
        *lenArrA = (*lenArrA) - 1;
    }
    *arrA = (char*)malloc(((*lenArrA) + 1) * sizeof(char));
    for(int i = 0; i < (*lenArrA) + 1; i++)
        (*arrA)[i] = strBuffA[i];

    fgets(strBuffB, MAX_ARRAY_SIZE, file);
    *lenArrB = strlen(strBuffB);
    if(strBuffB[(*lenArrB) - 1] == '\n')
    {
        strBuffB[(*lenArrB) - 1] = '\0';
        *lenArrB = (*lenArrB) - 1;
    }
    *arrB = (char*)malloc(((*lenArrB) + 1) * sizeof(char));
    for(int i = 0; i < (*lenArrB) + 1; i++)
        (*arrB)[i] = strBuffB[i];

    return 0;
}

void main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("You have to specify the type of execution, path to the test file, and desired number of threads!\n"); 
        printf("Example (if your executable is named \"lcs\"):\n   ./lcs parallel ../test_examples/test1 6 - for parallel execution with 6 threads\n");
        printf("   ./lcs _any_string_ ../test_examples/test1 6 - for sequential execution (only the path argument has an effect)\n");
        return;
    }

    int desiredNumOfThreads = atoi(argv[3]);
    if(desiredNumOfThreads <= 0)
    {
        printf("Number of threads has to be an integer greater than 0!\n");
        return;
    }

    srand(time(0));
    char* arrA;
    char* arrB;
    uint32_t lenArrA;
    uint32_t lenArrB;
    uint32_t htPow;

    if(_read_lcs_args_from_file(argv[2], &htPow, &lenArrA, &lenArrB, &arrA, &arrB) > 0)
        return;
    printf("Table size: 2^%d\nArray A length: %d\nArray B length: %d\nArray A: %s\nArray B: %s\n\n", htPow, lenArrA, lenArrB, arrA, arrB);


    ht* table1 = ht_create(_primary_hash_function_fib, _secondary_hash_function, htPow);
    ht* table = ht_create(_primary_hash_function_fib, _secondary_hash_function, htPow);
    ht* table2 = ht_create(_primary_hash_function_fib, _secondary_hash_function, htPow);


    double start;
    double stop;
    test = 0;
    if(strcmp(argv[1], "parallel"))
    {
        start = omp_get_wtime();
        int lcsLen1 = findLcs(arrA, arrB, lenArrA, lenArrB, table1);
        stop = omp_get_wtime();

        printf("Sequential execution time: %lf\n", stop-start);
        printf("\nLCS length: %d\n", lcsLen1);
    }   
    else
    {
        int numThreads;
        start = omp_get_wtime();
        #pragma omp parallel num_threads(desiredNumOfThreads)
        {
            #pragma omp single
            {
                numThreads = omp_get_num_threads();
                printf("Number of threads: %d\n", numThreads);
                fflush(stdout);
                for (int i = 0; i < numThreads; i++)
                {
                    #pragma omp task
                    {
                        int lcsLen = findLcs(arrA, arrB, lenArrA, lenArrB, table);
                        #ifdef TEST_EXIT_CONDITION
                        __sync_val_compare_and_swap(&test, 0, 1);
                        #endif
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

    free(arrA);
    free(arrB);
}