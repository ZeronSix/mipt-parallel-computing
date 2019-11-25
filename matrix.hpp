//
// Created by Vyacheslav Zhdanovskiy <zeronsix@gmail.com> on 11/21/19.
//
#pragma once

#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

template<typename T>
class Matrix
{
public:
    Matrix(std::size_t rows, std::size_t columns):
        m_rows{rows},
        m_columns{columns},
        m_memory{reinterpret_cast<T *>(std::calloc(rows * columns, sizeof(T)))}
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

                 ~Matrix()                       noexcept { std::free(m_memory); }
    T           *operator[](std::size_t y)       noexcept { return m_memory + m_columns * y; }
    T           *operator[](std::size_t y) const noexcept { return m_memory + m_columns * y; }
    std::size_t GetRows()                  const noexcept { return m_rows; }
    std::size_t GetColumns()               const noexcept { return m_columns; }

    void Resize(std::size_t w, std::size_t h)
    {
       m_rows = h;
       m_columns = w;
       auto buf = std::realloc(m_memory, sizeof(T) * w * h);
       if (!buf) throw std::runtime_error("realloc");
       m_memory = reinterpret_cast<T *>(buf);
    }
private:
    std::size_t m_rows;
    std::size_t m_columns;
    T           *m_memory;
};

template<typename T>
std::ostream &operator<<(std::ostream &stream, const Matrix<T> &matrix)
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
