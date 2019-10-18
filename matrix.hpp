#pragma once

#include <cstddef>
#include <stdexcept>

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
private:
    std::size_t m_rows;
    std::size_t m_columns;
    double      *m_memory;
};