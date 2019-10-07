//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <set>
#include <fstream>
#include <omp.h>
#include <iostream>
#include "Field.h"

Field::Field(int width, int height):
    m_memory{static_cast<bool *>(std::calloc(width * height,
                                             sizeof(*m_memory)))},
    m_width{width},
    m_height{height},
    m_isGameOver{false},
    m_prevChangesByThread{static_cast<std::size_t>(omp_get_max_threads())}
{
    assert(width > 0);
    assert(height > 0);

    if (!m_memory)
    {
        throw std::runtime_error("Failed to allocate memory");
    }
}

Field::Field(Field &&other) noexcept:
    m_memory{other.m_memory},
    m_width{other.m_width},
    m_height{other.m_height},
    m_isGameOver{other.m_isGameOver},
    m_prevChangesByThread{other.m_prevChangesByThread}
{
    other.m_memory = nullptr;
}

Field::~Field()
{
    std::free(m_memory);
}

sf::Vector2i Field::GetSize() const
{
    return sf::Vector2i{m_width, m_height};
}

void Field::Update()
{
    static constexpr std::pair<int, int> neighbourOffsets[] = {{-1, -1},
                                                               {-1, 0},
                                                               {-1, 1},
                                                               {0, -1},
                                                               {0, 1},
                                                               {1, -1},
                                                               {1, 0},
                                                               {1, 1}};
    ChangesByThread cellsToChangeByThread{static_cast<size_t>(32)};

#pragma omp parallel default(none) shared(cellsToChangeByThread, \
                                          neighbourOffsets)
    {
#pragma omp for
        for (int y = 0; y < m_height; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                int aliveCount = std::count_if(std::begin(neighbourOffsets),
                                               std::end(neighbourOffsets),
                                               [this, x, y](std::pair<int, int> offset)
                                               {
                                                   return At(x + offset.first, y + offset.second);
                                               });

                // If the cell is dead and there are 3 neighbours, the cell becomes alive
                // OR
                // If the cell is alive and there are neither 2 nor 3 neighbours, the cell dies
                if ((!At(x, y) && aliveCount == 3) ||
                    (At(x, y) && (aliveCount != 2 && aliveCount != 3)))
                {
                    cellsToChangeByThread[omp_get_thread_num()].push_back({x, y});
                }
            }
        }

#pragma omp for
        // OpenMP doesn't support range-based for loops
        for (int i = 0; i < omp_get_max_threads(); i++)
        {
            for (std::size_t j = 0; j < cellsToChangeByThread[i].size(); j++)
            {
                auto cell = cellsToChangeByThread[i][j];
                At(cell.first, cell.second) = !At(cell.first, cell.second);
            }
        }
    }

    if (cellsToChangeByThread == m_prevChangesByThread)
    {
        m_isGameOver = true;
    }
    m_prevChangesByThread = cellsToChangeByThread;
}

static int CycleClipCoord(int val, int range)
{
    if (val < 0)
    {
        return ((std::abs(val) / range + 1) * range + val) % range;
    }
    else
    {
        return val % range;
    }
}

const bool &Field::At(int x, int y) const
{
    x = CycleClipCoord(x, m_width);
    y = CycleClipCoord(y, m_height);

    assert(0 <= x && x < m_width);
    assert(0 <= y && y < m_height);

    return m_memory[y * m_width + x];
}

bool &Field::At(int x, int y)
{
    x = CycleClipCoord(x, m_width);
    y = CycleClipCoord(y, m_height);

    assert(0 <= x && x < m_width);
    assert(0 <= y && y < m_height);

    return m_memory[y * m_width + x];
}

Field Field::LoadFromFile(const std::string &fileName)
{
    if (fileName.empty())
    {
        return Field{DEFAULT_WIDTH, DEFAULT_HEIGHT};
    }

    std::ifstream ifs{fileName};
    if (!ifs.is_open())
    {
        throw std::runtime_error("Failed to open " + fileName);
    }

    int w = 0, h = 0;
    if (!(ifs >> w >> h))
    {
        throw std::runtime_error("Failed to parse input!");
    }

    Field field{w, h};
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (!(ifs >> field.At(x, y)))
            {
                throw std::runtime_error("Failed to parse input!");
            }
        }
    }

    return field;
}

bool Field::IsGameOver() const
{
    return m_isGameOver;
}
