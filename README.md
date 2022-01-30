# parallel-lcs
A dynamic programming parallel implementation of the LCS problem

# How to compile using gcc
- set the current working directory to ./source
- "gcc ht.c lcs.c -o name_of_the_executable -fopenmp"

## TEST_EXIT_CONDITION macro
If you compile the program without defining the TEST_EXIT_CONDITION macro, the recursive calls to the findLcs function will not check the global "test" variable, which indicates that one of the threads has already found the solution and the other should stop their execution. Defining the macro, however, will not always lead to a significant decrease in the execution time, because, when one thread finishes, the hashtable used for memoization will have the solution to most of the subproblems stored, if not all of them. Other threads will just read the solutions and finish their job not much after the first one. This macro only affects the parallel version of the program.

### How to define the macro while compiling
- "gcc ht.c lcs.c -o name_of_the_executable -fopenmp -D TEST_EXIT_CONDITION"

# Running the program
- ./name_of_the_executable _option1_ _option2_ _option3_
Options are:
1. _option1_ - "parallel" for parallel execution; any other string for sequential exeuction
2. _option2_ - path to the file with arguments; you can find the required format of the file content furhter bellow; you can also use one of the four already existing test files from the "test_examples" directory
3. _option3_ - an integer bigger than zero, specifying the number of threads; only affects the parallel version

# Format of the file with arguments
The file with the arguments must contain three arguments, all writen in separate lines, in the following order:
1. The power to which number two will be raised in order to calculate the hashtable size
2. The first array
3. The second array

For example, if the content of the file is:
4
ajweql
adsjdakl

then the program will use a hastable with 2^4 entries in order to calculate the length of the longest common subsequence of arrays "ajweql" and "adsjdakl". The file must not have any leading white lines, but can have trailing ones.
