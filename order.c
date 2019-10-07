#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Wrong usage!\n");
        return EXIT_FAILURE;
    }

    long n = strtol(argv[1], NULL, 10);
    omp_set_num_threads(n);
    int i = 0;
    #pragma omp parallel shared(i)
    {
        int thread_id = omp_get_thread_num();
        while (i != thread_id)
        {
        }

        printf("I am %d, i = %d\n", thread_id, i);
        i++;
    }

    return 0;
}