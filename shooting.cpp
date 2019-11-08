#include <mpi.h>
#include <cstddef>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <fstream>

constexpr std::size_t POINTS{10000};
constexpr std::size_t INTERVALS{POINTS - 1};
constexpr double      LEFT{0.0};
constexpr double      RIGHT{3 * M_PI};
constexpr double      Y_LEFT{0.0};
constexpr double      Y_RIGHT{-2.0};
constexpr double      STEP{(RIGHT - LEFT) / INTERVALS};
constexpr std::size_t MAX_PROCESSES = 100;

double f(double t) 
{
    return std::sin(t);
}

struct Task
{
    std::size_t numOfIntervals;
    double left;
    double dy0;
    double *y;
    double a;
    double b;
    double beta;

    void Compute()
    {
        y[0] = a;
        y[1] = y[0] + dy0 * STEP;
        for (std::size_t i = 2; i < numOfIntervals; ++i)
        {
            y[i] = YFromDiff2(i);
        }

        b = YFromDiff2(numOfIntervals);
        beta = (y[numOfIntervals - 1] - y[numOfIntervals - 2]) / STEP;
    }

    double YFromDiff2(std::size_t i)
    {
        return f(left + (i - 1) * STEP) * STEP * STEP +
               2 * y[i - 1] - y[i - 2];
    }

    double GetIntervalSize() const
    {
        return numOfIntervals * STEP;
    }
};

int main(int argc, char *argv[])
{
    Task tasks[MAX_PROCESSES] = {};

    MPI_Init(nullptr, nullptr);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    std::size_t intervalsPerProcess = std::ceil(1.0 * INTERVALS / size);
    double *y = reinterpret_cast<double *>(std::calloc(intervalsPerProcess, sizeof(*y)));
    if (!y) throw std::runtime_error("Calloc");
    y[0] = Y_LEFT;

    std::size_t offset = rank * intervalsPerProcess;
    Task task = {};
    task.numOfIntervals = (rank == size - 1) ? INTERVALS - rank * intervalsPerProcess :
                                               intervalsPerProcess;
    task.left = LEFT + offset * STEP;
    task.dy0 = 0;
    task.y = y;
    task.a = 0;

    double startTime = MPI_Wtime();
    task.Compute();

    Task *taskBuf = (rank == 0) ? tasks : nullptr;
    MPI_Gather(&task, sizeof(Task), MPI_BYTE, taskBuf, sizeof(Task), MPI_BYTE, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        double b = tasks[0].b;
        double betaSum = 0;
        for (int i = 1; i < size; i++)
        {
            betaSum += tasks[i - 1].beta;
            b += tasks[i].b + betaSum * tasks[i].GetIntervalSize();
        }
        tasks[0].dy0 = (Y_RIGHT - b) / (RIGHT - LEFT);

        for (int i = 1; i < size; i++)
        {
            tasks[i].a = tasks[i - 1].a + tasks[i - 1].dy0 * tasks[i - 1].GetIntervalSize() + tasks[i - 1].b;
            tasks[i].dy0 = tasks[i - 1].dy0 + tasks[i - 1].beta;
        }
    }
    MPI_Scatter(tasks, sizeof(Task), MPI_BYTE, &task, sizeof(Task), MPI_BYTE, 0, MPI_COMM_WORLD);
    task.Compute();

    double *yRcvBuf = nullptr;
    if (rank == 0)
    {
        yRcvBuf = reinterpret_cast<double *>(std::calloc(intervalsPerProcess * size, sizeof(*y)));
    }
    MPI_Gather(y, intervalsPerProcess, MPI_DOUBLE, yRcvBuf, intervalsPerProcess, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double endTime = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Computation time: " << endTime - startTime << std::endl;

        std::ofstream stream{"plot.csv"};
        stream << "x,y" << std::endl;
        double x = 0;
        for (std::size_t i = 0; i < INTERVALS; ++i)
        {
            stream << x << ',' << yRcvBuf[i] << std::endl;
            x += STEP;
        }
    }

    std::free(yRcvBuf);
    std::free(y);
    MPI_Finalize();
    return 0;
}
