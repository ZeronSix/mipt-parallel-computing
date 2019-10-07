#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUMBER_OF_TERMS 1000000000
#define INTERVALS 10000
#define TERMS_PER_INTEVAL (NUMBER_OF_TERMS / INTERVALS)

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage %s num of threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n = strtol(argv[1], NULL, 10);
    omp_set_num_threads(n);
    
    double local_sums[INTERVALS] = {0.0};
    for (int i = 0; i < INTERVALS; i++)
    {
        local_sums[i] = 0.0f;
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < INTERVALS; i++)
        {
            double local_sum = 0.0;
            for (long j = 1; j <= TERMS_PER_INTEVAL; j++)
            {
                local_sum += 1.0 / (i * TERMS_PER_INTEVAL + j);
            }
            local_sums[i] += local_sum;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < INTERVALS; i++)
    {
        sum += local_sums[i];
    }

    printf("%.20lg\n", sum);
    return 0;
}