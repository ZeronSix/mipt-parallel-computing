#include <mpi.h>
#include <cstdio>
#include <cmath>
#include <string>
#include "matrix.hpp"

constexpr int ISIZE{1000};
constexpr int JSIZE{1000};

void ProcessDiagonal(Matrix &mat, int diagonal)
{
    for (int y = 0; y < ISIZE; ++y) 
    {
        int x = diagonal - y;
        if (x < 0 || y < 1 || x >= JSIZE - 1 || y >= ISIZE) continue;
        mat[y][x] = std::sin(0.00001 * mat[y - 1][x + 1]);
    }
}

void SendDiagonal(Matrix &mat, int diagonal)
{
    for (int y = 0; y < ISIZE; ++y) 
    {
        int x = diagonal - y;
        if (x < 0 || y < 0 || x >= JSIZE || y >= ISIZE) continue;
        double data[3] = {double(y), double(x), mat[y][x]};
        MPI_Send(data, 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[])
{
    Matrix a{ISIZE, JSIZE};

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
        }
    }

    double startTime = MPI_Wtime();
    int elementsProcessed = 0;
    for (int i = rank; i < ISIZE; i += processCount)
    {
        ProcessDiagonal(a, i);
        elementsProcessed += i + 1;
        if (i != ISIZE - 1)
        {
            // Do not process the middle diagonal twice
            ProcessDiagonal(a, ISIZE + i);
            elementsProcessed += ISIZE - i - 1;
        }
        std::printf("%d - %d\n", i, ISIZE + i);
    }
    double endTime = MPI_Wtime();
    std::printf("Process %d, calc time %f\n", rank, endTime - startTime);
    std::fprintf(stderr, "ELS proc %d\n", elementsProcessed);

    
    if (rank != 0)
    {
        for (int i = rank; i < ISIZE; i += processCount)
        {
            SendDiagonal(a, i);
            if (i != ISIZE - 1)
            {
                SendDiagonal(a, ISIZE + i);
            }
        }
    }
    else 
    {
        while (elementsProcessed != ISIZE * JSIZE)
        {
            double data[3] = {};
            MPI_Recv(data, 3, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            a[int(data[0])][int(data[1])] = data[2];
            elementsProcessed++;
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