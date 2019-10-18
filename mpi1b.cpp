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
    Matrix copy{ISIZE, JSIZE};

    MPI_Init(NULL, NULL);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int processCount = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);
    
    for (int i = 0; i < ISIZE; ++i)
    {
        for (int j = 0; j < JSIZE; ++j)
        {
            a[i][j] = 10 * i + j;
            copy[i][j] = a[i][j];
        }
    }

    double startTime = MPI_Wtime();
    int rowsPerProcess = std::ceil(1.0f * ISIZE / processCount);
    int start = rank * rowsPerProcess;
    int end = (rank == processCount - 1) ? ISIZE - 3 : start + rowsPerProcess;
    for (int i = start; i < end; ++i)
    {
        for (int j = 4; j < JSIZE - 1; ++j)
        {
            a[i][j] = std::sin(0.00001 * copy[i + 3][j - 4]);
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
        for (int i = 1; i < processCount; ++i)
        {
            start = i * rowsPerProcess;
            end = (i == processCount - 1) ? ISIZE : start + rowsPerProcess;
            MPI_Recv(a[start], (end - start) * JSIZE, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::string filename = "result_1b_" + std::to_string(processCount) + ".txt";
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