//
// Created by Vyacheslav Zhdanovskiy <zeronsix@gmail.com> on 11/21/19.
//
#pragma once

#include <algorithm>
#include <mutex>
#include <omp.h>
#include "matrix.hpp"
#include "heatmap.hpp"

struct ThreadCtl
{
    bool running;
    bool stop;
    int iteration;
    std::mutex mutex;
};

enum class Method
{
    Jacobi,
    Seidel
};

struct TaskParams
{
    std::size_t threads;
    std::size_t width;
    std::size_t height;

    Method method;

    float left;
    float right;
    float top;
    float bottom;
    float eps;

    TaskParams():
        threads{1},
        width{START_WIDTH},
        height{START_HEIGHT},
        method{Method::Jacobi},
        left{100},
        right{200},
        top{300},
        bottom{400},
        eps{1e-3} {}

    float GetMin() const { return std::min({left, right, top, bottom});}
    float GetMax() const { return std::max({left, right, top, bottom});}
    float GetAvg() const { return (left + right + top + bottom) / 4; }
};

class Heatmap;

class Solver
{
public:
    Solver(const TaskParams &params, Heatmap &heatmap, ThreadCtl &ctl):
        m_params{params},
        m_heatmap{heatmap},
        m_ctl{ctl} {}

    void Solve();
    void SolveJacobi(Matrix<float> &m);
    void SolveSeidel(Matrix<float> &m);
private:
    TaskParams m_params;
    Heatmap    &m_heatmap;
    ThreadCtl  &m_ctl;
};

void Solver::Solve()
{
    Matrix<float> m{m_params.height, m_params.width};
    const float avg = m_params.GetAvg();
    const float minTemp = m_params.GetMin();
    const float maxTemp = m_params.GetMax();
    if (maxTemp == minTemp) return;

    for (auto y = 0ul; y < m.GetRows(); ++y)
    {
        for (auto x = 0ul; x < m.GetColumns(); ++x)
        {
            float val = avg;
            if (x == 0) val = m_params.left;
            if (x == m_params.width - 1) val = m_params.right;
            if (y == 0) val = m_params.top;
            if (y == m_params.height - 1)
                val = m_params.bottom;

            m[y][x] = val;
            if (minTemp != maxTemp)
            {
                m_heatmap.Set(x,
                              y,
                              val,
                              avg,
                              minTemp,
                              maxTemp);
            }
        }
    }

    omp_set_num_threads(m_params.threads);
    if (m_params.method == Method::Jacobi)
    {
        SolveJacobi(m);
    }
    else
    {
        SolveSeidel(m);
    }

    std::lock_guard<std::mutex> guard{m_ctl.mutex};
    m_ctl.running = false;
    m_ctl.stop = false;
}

void Solver::SolveJacobi(Matrix<float> &m)
{
    Matrix<float> copy{m};
    const float avg = m_params.GetAvg();
    const float minTemp = m_params.GetMin();
    const float maxTemp = m_params.GetMax();

    auto next = &m;
    auto prev = &copy;

    for (;;)
    {
        bool converged = true;
        #pragma omp parallel for schedule(static) default(shared)
        for (std::size_t y = 1; y < m_params.height - 1; ++y)
        {
            for (std::size_t x = 1; x < m_params.width - 1; ++x)
            {
                (*next)[y][x] = ((*prev)[y][x - 1] +
                    (*prev)[y][x + 1] +
                    (*prev)[y - 1][x] +
                    (*prev)[y + 1][x]) / 4;

                m_heatmap.Set(x,
                              y,
                              (*next)[y][x],
                              avg,
                              minTemp,
                              maxTemp);

                if (converged && std::abs((*next)[y][x] - (*prev)[y][x]) > m_params.eps)
                {
                    converged = false;
                }
            }
        }

        if (converged || m_ctl.stop) break;
        m_ctl.iteration++;
        std::swap(next, prev);
    }
}

void Solver::SolveSeidel(Matrix<float> &m)
{
    const float avg = m_params.GetAvg();
    const float minTemp = m_params.GetMin();
    const float maxTemp = m_params.GetMax();

    for (;;)
    {
        bool converged = true;
        int counter = m_params.threads;
        #pragma omp parallel default(shared)
        {
            bool sync = false;
            #pragma omp for schedule(static)
            for (std::size_t y = 1; y < m_params.height - 1; ++y)
            {
                if (omp_get_thread_num() <= m_ctl.iteration)
                {
                    for (std::size_t x = 1; x < m_params.width - 1; ++x)
                    {
                        float prev = m[y][x];
                        m[y][x] = (m[y][x - 1] + m[y][x + 1] + m[y - 1][x] + m[y + 1][x]) / 4;

                        m_heatmap.Set(x,
                                      y,
                                      m[y][x],
                                      avg,
                                      minTemp,
                                      maxTemp);

                        if (converged && std::abs(m[y][x] - prev) > m_params.eps)
                        {
                            converged = false;
                        }
                    }
                }
            }
        }

        if (converged || m_ctl.stop) break;
        m_ctl.iteration++;
    }
}
