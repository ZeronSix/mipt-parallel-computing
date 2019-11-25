//
// Created by Vyacheslav Zhdanovskiy <zeronsix@gmail.com> on 11/22/19.
//
#pragma once

#ifndef HEATMAP_HPP
#define HEATMAP_HPP

#include <cstddef>
#include <SFML/Graphics.hpp>
#include "matrix.hpp"

constexpr std::size_t START_WIDTH{100};
constexpr std::size_t START_HEIGHT{100};

struct Pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

constexpr Pixel MIN_COLOR{0, 0, 255, 255};   // blue
constexpr Pixel MAX_COLOR{255, 0, 0, 255}; // red
constexpr Pixel MID_COLOR{255, 255, 0, 255}; // yellow

Pixel Lerp(const Pixel &a, const Pixel &b, float c)
{
    if (c > 1.0f) c = 1.0f;
    return Pixel{static_cast<uint8_t>(a.r + (b.r - a.r) * c),
                 static_cast<uint8_t>(a.g + (b.g - a.g) * c),
                 static_cast<uint8_t>(a.b + (b.b - a.b) * c),
                 static_cast<uint8_t>(a.a + (b.a - a.a) * c)};
}

class Heatmap
{
public:
    Heatmap():
        m_pixels{START_WIDTH, START_HEIGHT}
    {
        if (!m_texture.create(START_HEIGHT, START_WIDTH)) throw std::runtime_error("create texture");
        m_texture.setSmooth(true);
        m_sprite.setTexture(m_texture);

        for (auto y = 0ul; y < START_HEIGHT; ++y)
        {
            for (auto x = 0ul; x < START_WIDTH; ++x)
            {
                m_pixels[y][x] = MIN_COLOR;
            }
        }
    }
    void Set(std::size_t x, std::size_t y, float value, float avg, float min, float max) noexcept
    {
        if (value < avg)
        {
            m_pixels[y][x] = Lerp(MIN_COLOR, MID_COLOR, (value - min) / (avg - min));
        }
        else
        {
            m_pixels[y][x] = Lerp(MID_COLOR, MAX_COLOR, (value - avg) / (max - avg));
        }
    }
    void UpdateTexture()
    {
        m_texture.update(reinterpret_cast<const sf::Uint8 *>(&m_pixels[0][0]));
    }
    void Resize(std::size_t w, std::size_t h)
    {
        m_pixels.Resize(w, h);
        m_texture.create(w, h);
        UpdateTexture();
        m_sprite.setTexture(m_texture);
        m_sprite.setTextureRect(sf::IntRect(0, 0, w, h));
    }
    void Render(sf::RenderTarget &target) { target.draw(m_sprite); };
    void FillScreen(const sf::RenderTarget &target)
    {
        m_sprite.setScale(target.getSize().x / m_sprite.getLocalBounds().width,
                          target.getSize().y / m_sprite.getLocalBounds().height);
    }
private:
    std::size_t   m_width, m_height;
    sf::Texture   m_texture;
    sf::Sprite    m_sprite;
    Matrix<Pixel> m_pixels;
};

#endif //HEATMAP_HPP
