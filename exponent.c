#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUMBER_OF_TERMS 1000000000
#define MAX_THREADS 128

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage %s num of threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n = strtol(argv[1], NULL, 10);
    omp_set_num_threads(n);
    
    double sum = 1.0;
    double global_fact = 1.0;
    #pragma omp parallel
    {
        double local_sum = 0.0;
        double fact = 1.0;
        
        #pragma omp for
        for (long i = 1; i < NUMBER_OF_TERMS; i++)
        {
            fact *= 1.0 / i;
            local_sum += fact;
        }

        #pragma omp for ordered
        for (int i = 0; i < omp_get_num_threads(); i++)
        {
            #pragma omp ordered
            {
                sum += local_sum * global_fact;
                global_fact *= fact;
            }
        }
    }

    printf("%.20lg\n", sum);
    return 0;
}