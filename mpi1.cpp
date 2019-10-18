#include <mpi.h>
#include <cstdio>
#include <cmath>
#include <string>
#include "matrix.hpp"

constexpr int ISIZE{1000};
constexpr int JSIZE{1000};

int main(int argc, char *argv[])
{
    Matrix a{ISIZE, JSIZE};

    MPI_Init(NULL, NULL);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    for (int i = 0; i < ISIZE; ++i)
    {
        for (int j = 0; j < JSIZE; ++j)
        {
            a[i][j] = 10 * i + j;
        }
    }

    double startTime = MPI_Wtime();
    int rowsPerProcess = std::ceil(1.0f * ISIZE / size);
    int start = rank * rowsPerProcess;
    int end = (rank == size - 1) ? ISIZE : start + rowsPerProcess;
    for (int i = start; i < end; ++i)
    {
        for (int j = 0; j < JSIZE; ++j)
        {
            a[i][j] = std::sin(0.00001 * a[i][j]);
        }
    }
    double endTime = MPI_Wtime();
    std::printf("Process %d, calc time %f\n", rank, endTime - startTime);

    if (rank != 0)
    {
        MPI_Send(a[start], (end - start) * JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else 
    {
        for (int i = 1; i < size; ++i)
        {
            start = i * rowsPerProcess;
            end = (i == size - 1) ? ISIZE : start + rowsPerProcess;
            MPI_Recv(a[start], (end - start) * JSIZE, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::string filename = "result_1_" + std::to_string(size) + ".txt";
        std::FILE *ff = std::fopen(filename.c_str(), "w");
        for (int i = 0; i < ISIZE; ++i)
        {
            for (int j = 0; j < JSIZE; ++j)
            {
                std::fprintf(ff, "%f ", a[i][j]);
            }
            std::fprintf(ff, "\n");
        }
        std::fclose(ff);
    }

    MPI_Finalize();

    return 0;
}