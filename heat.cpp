#include <mpi.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <fstream>
#include "matrix.hpp"
#include "solvers.hpp"

constexpr int YSIZE{200};
constexpr int XSIZE{200};

constexpr double TEMP_LEFT{100.0};
constexpr double TEMP_RIGHT{300.0};
constexpr double TEMP_TOP{200.0};
constexpr double TEMP_BOTTOM{400.0};

enum class method_t
{
    JACOBI,
    SEIDEL
};

int main(int argc, char *argv[])
{
    MPI_Init(nullptr, nullptr);

    Matrix matrix{YSIZE, XSIZE};

    for (std::size_t x = 0; x < XSIZE; ++x)
    {
        matrix[0][x] = TEMP_TOP;
        matrix[YSIZE - 1][x] = TEMP_BOTTOM;
    }

    for (std::size_t y = 0; y < YSIZE; ++y)
    {
        matrix[y][0] = TEMP_LEFT;
        matrix[y][XSIZE - 1] = TEMP_RIGHT;
    }

    double avg = (TEMP_LEFT + TEMP_RIGHT + TEMP_TOP + TEMP_BOTTOM) / 4;
    for (std::size_t y = 1; y < YSIZE - 1; ++y)
    {
        for (std::size_t x = 1; x < XSIZE - 1; ++x)
        {
            matrix[x][y] = avg;
        }
    }


    Solver solver;

    double startTime = MPI_Wtime();
    Matrix result{solver.SolveSeidel(matrix)};
    double endTime = MPI_Wtime();

    if (solver.GetRank() == solver.GetProcessCount() - 1)
    {
        std::string filename = "result_" + std::to_string(solver.GetProcessCount()) + ".txt";
        std::ofstream stream{filename, std::ios::out};
        if (!stream.is_open()) throw std::runtime_error{"Failed to open file " + filename};
        stream << result;

        std::printf("Converged in %d iterations, took %f seconds\n", solver.GetIterations(), endTime - startTime);
    }

    MPI_Finalize();

    return 0;
}