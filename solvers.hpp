#pragma once

#include "matrix.hpp"

constexpr double EPS{1e-4};

enum tag_t
{
    TAG_MSG,
    TAG_DATA
};

enum message_t
{
    MSG_SEND,
    MSG_CONVERGED,
    MSG_NOT_CONVERGED,
    MSG_CONTINUE,
    MSG_STOP
};

class Solver
{
public:
           Solver();
    Matrix SolveJacobi(Matrix &original);
    Matrix SolveSeidel(Matrix &original);

    int    GetRank()         const { return m_rank; }
    int    GetProcessCount() const { return m_size; }
    int    GetIterations()   const { return m_iter; }
private:
    int m_rank;
    int m_size;
    int m_iter;

    MPI_Request m_requests[4] = {};

    std::size_t GetStartY(std::size_t rows) const;
    std::size_t GetSizeY(std::size_t rows) const;
    void        SendMessage(int rank, message_t msg) const;
    message_t   RecvMessage(int rank) const;
    message_t   BroadcastMessage(message_t msg, int root) const;
    bool        CheckConvergenceOf(int rank) const;
    bool        CheckConvergenceOfTheRest()  const;
    void        SendMatrixBlockAsync(const Matrix &mat, int rank, int startRow, int rows, int req);
    void        RecvMatrixBlockAsync(Matrix &mat, int rank, int startRow, int rows, int req);
    void        SendMatrixBlock(const Matrix &mat, int rank, int startRow, int rows);
    void        RecvMatrixBlock(Matrix &mat, int rank, int startRow, int rows);
    void        WaitForRequests();
}; 

inline Solver::Solver()
{
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_size);
    m_iter = 0;
}

inline Matrix Solver::SolveJacobi(Matrix &original)
{
    auto startY  = GetStartY(original.GetRows());
    auto sizeY   = GetSizeY(original.GetRows());
    auto sizeX   = original.GetColumns();
    auto maxSizeY = static_cast<std::size_t>(std::ceil(1.0f * (original.GetRows() - 2) / m_size));
    auto bufRows = maxSizeY * m_size + 2;
    Matrix copy1{original, bufRows};
    Matrix copy2{original, bufRows};
    Matrix *next = &copy1;
    Matrix *prev = &copy2;

    Matrix result{original, bufRows};

    bool converged = true;
    m_iter = 0;
    do 
    {
        converged = true;

        if (m_rank < m_size - 1) RecvMatrixBlockAsync(*next, m_rank + 1, startY + sizeY, 1, 2);
        if (m_rank > 0) RecvMatrixBlockAsync(*next, m_rank - 1, startY - 1, 1, 3);
        for (std::size_t y = startY; y < startY + sizeY; ++y)
        {
            for (std::size_t x = 1; x < sizeX - 1; ++x)
            {
                (*next)[y][x] = ((*prev)[y][x - 1] +
                                 (*prev)[y][x + 1] +
                                 (*prev)[y - 1][x] +
                                 (*prev)[y + 1][x]) / 4;

                if (std::abs((*next)[y][x] - (*prev)[y][x]) > EPS) 
                {
                    converged = false;
                }
            }

            if (y == startY && m_rank > 0) SendMatrixBlockAsync(*next, m_rank - 1, startY, 1, 1);
        }

        if (m_rank < m_size - 1) SendMatrixBlockAsync(*next, m_rank + 1, startY + sizeY - 1, 1, 0);
        WaitForRequests();
        std::swap(next, prev);

        if (m_rank == 0)
        {
            bool rest = CheckConvergenceOfTheRest();
            converged = converged && rest;
        }
        else
        {
            message_t msg = converged ? MSG_CONVERGED : MSG_NOT_CONVERGED;
            SendMessage(0, msg);
        }

        message_t contMsg = converged ? MSG_STOP : MSG_CONTINUE;
        converged = BroadcastMessage(contMsg, 0) == MSG_STOP;
        m_iter++;
    } while (!converged);


    //printf("%d %lu %lu %d\n", m_rank, startY, startY + sizeY, iter);
    MPI_Gather((*prev)[startY], maxSizeY * result.GetColumns(), MPI_DOUBLE, 
               result[1], maxSizeY * result.GetColumns(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return result;
}

inline Matrix Solver::SolveSeidel(Matrix &original)
{
    auto startY  = GetStartY(original.GetRows());
    auto sizeY   = GetSizeY(original.GetRows());
    auto sizeX   = original.GetColumns();
    auto maxSizeY = static_cast<std::size_t>(std::ceil(1.0f * (original.GetRows() - 2) / m_size));
    auto bufRows = maxSizeY * m_size + 2;
    Matrix copy{original, bufRows};
    Matrix result{original, bufRows};

    bool converged = true;
    bool restConverged = false;
    m_iter = 0;
    do 
    {
        converged = true;
        if (m_rank > 0 && !restConverged)
        {
            restConverged = CheckConvergenceOf(m_rank - 1);
            //if (rest) std::cerr << "TEST\n";
            converged = converged && restConverged;
            RecvMatrixBlock(copy, m_rank - 1, startY - 1, 1);
        }

        for (std::size_t y = startY; y < startY + sizeY; ++y)
        {
            for (std::size_t x = 1; x < sizeX - 1; ++x)
            {
                double old = copy[y][x];
                copy[y][x] = (copy[y][x - 1] +
                              copy[y][x + 1] +
                              copy[y - 1][x] +
                              copy[y + 1][x]) / 4;

                if (std::abs(copy[y][x] - old) > EPS) 
                {
                    converged = false;
                }
            }

            if (y == startY && m_rank > 0) SendMatrixBlock(copy, m_rank - 1, startY, 1);
        }

        if (m_rank < m_size - 1) 
        {
            message_t msg = converged ? MSG_CONVERGED : MSG_NOT_CONVERGED;
            SendMessage(m_rank + 1, msg);
            SendMatrixBlock(copy, m_rank + 1, startY + sizeY - 1, 1);
            RecvMatrixBlock(copy, m_rank + 1, startY + sizeY, 1);
        }

        message_t contMsg = converged ? MSG_STOP : MSG_CONTINUE;
        //converged = BroadcastMessage(contMsg, m_size - 1) == MSG_STOP;
        m_iter++;
    } while (!converged);


    //printf("%d %lu %lu %d\n", m_rank, startY, startY + sizeY, iter);
    MPI_Gather(copy[startY], maxSizeY * result.GetColumns(), MPI_DOUBLE, 
               result[1], maxSizeY * result.GetColumns(), MPI_DOUBLE, m_size - 1, MPI_COMM_WORLD);
    return result;
}

inline std::size_t Solver::GetStartY(std::size_t rows) const
{
    rows = rows - 2; // exclude the first and the last row
    return 1 + m_rank * static_cast<std::size_t>(std::ceil(1.0 * rows / m_size));
}

inline std::size_t Solver::GetSizeY(std::size_t rows) const
{
    return std::min(rows - 1 - GetStartY(rows), static_cast<std::size_t>(std::ceil(1.0f * (rows - 2) / m_size)));
}

inline void Solver::SendMessage(int rank, message_t msg) const
{
    MPI_Send(&msg, 1, MPI_INT, rank, TAG_MSG, MPI_COMM_WORLD);
}

inline message_t Solver::RecvMessage(int rank) const
{
    message_t msg = MSG_SEND;
    MPI_Recv(&msg, 1, MPI_INT, rank, TAG_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return msg;
}

inline message_t Solver::BroadcastMessage(message_t msg, int root) const
{
    MPI_Bcast(&msg, 1, MPI_INT, root, MPI_COMM_WORLD);
    return msg;
}

inline bool Solver::CheckConvergenceOf(int rank) const
{
    return RecvMessage(rank) == MSG_CONVERGED;
}

inline bool Solver::CheckConvergenceOfTheRest() const
{
    bool converged = true;
    for (int rank = 1; rank < m_size; ++rank)
    {
        if (!CheckConvergenceOf(rank))
        {
            converged = false;
        }
    }
    
    return converged;
}

inline void Solver::SendMatrixBlockAsync(const Matrix &mat, int rank, int startRow, int rows, int req)
{
    MPI_Isend(mat[startRow], rows * mat.GetColumns(), MPI_DOUBLE, rank, TAG_DATA, MPI_COMM_WORLD, &m_requests[req]);
}

inline void Solver::RecvMatrixBlockAsync(Matrix &mat, int rank, int startRow, int rows, int req)
{
    MPI_Irecv(mat[startRow], rows * mat.GetColumns(), MPI_DOUBLE, rank, TAG_DATA, 
             MPI_COMM_WORLD, &m_requests[req]);
}

inline void Solver::SendMatrixBlock(const Matrix &mat, int rank, int startRow, int rows)
{
    MPI_Send(mat[startRow], rows * mat.GetColumns(), MPI_DOUBLE, rank, TAG_DATA, MPI_COMM_WORLD);
}

inline void Solver::RecvMatrixBlock(Matrix &mat, int rank, int startRow, int rows)
{
    MPI_Recv(mat[startRow], rows * mat.GetColumns(), MPI_DOUBLE, rank, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
 
inline void Solver::WaitForRequests()
{
    for (int i = 0; i < 4; ++i)
    {
        if (!m_requests[i]) continue;
        MPI_Wait(&m_requests[i], MPI_STATUS_IGNORE);
    }
}