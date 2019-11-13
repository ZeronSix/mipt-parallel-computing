#pragma once

#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <mpi.h>

class Matrix
{
public:
    Matrix(std::size_t rows, std::size_t columns):
        m_rows{rows},
        m_columns{columns},
        m_memory{new double[rows * columns]}
    {
        if (!m_memory) throw std::runtime_error("Memory allocation");
    }

    Matrix(const Matrix &other):
        Matrix{other.m_rows, other.m_columns}
    {
        for (std::size_t i = 0; i < other.GetRows(); ++i)
        {
            for (std::size_t j = 0; j < other.GetColumns(); ++j)
            {
                (*this)[i][j] = other[i][j];
            }
        }
    }

    Matrix(const Matrix &other, std::size_t rowsInBuffer):
        m_rows{other.m_rows},
        m_columns{other.m_columns},
        m_memory{new double[rowsInBuffer * other.m_columns]}
    {
        for (std::size_t i = 0; i < other.GetRows(); ++i)
        {
            for (std::size_t j = 0; j < other.GetColumns(); ++j)
            {
                (*this)[i][j] = other[i][j];
            }
        }
    }


    ~Matrix() noexcept
    {
        delete[] m_memory;
    }

    double *operator[](std::size_t y) noexcept
    {
        return m_memory + m_columns * y;
    }

    double *operator[](std::size_t y) const noexcept
    {
        return m_memory + m_columns * y;
    }

    std::size_t GetRows() const noexcept 
    {
        return m_rows;
    }

    std::size_t GetColumns() const noexcept 
    {
        return m_columns;
    }
private:
    std::size_t m_rows;
    std::size_t m_columns;
    double      *m_memory;
};

inline std::ostream &operator<<(std::ostream &stream, const Matrix &matrix)
{
    for (std::size_t i = 0; i < matrix.GetRows(); ++i)
    {
        for (std::size_t j = 0; j < matrix.GetColumns(); ++j)
        {
            stream << matrix[i][j] << ' ';
        }
        stream << std::endl;
    }

    return stream;
}
