//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#include <cmath>
#include <iostream>
#include "FieldRenderer.h"

FieldRenderer::FieldRenderer(Field &field, sf::RenderTarget &target):
    m_field{field},
    m_target{target},
    m_cell{sf::RectangleShape{sf::Vector2f{CELL_SIZE, CELL_SIZE}}}
{
    m_cell.setFillColor(sf::Color::Black);

    CreateGrid();
}

void FieldRenderer::Render()
{
    auto view = m_target.getView();
    auto center = view.getCenter();
    auto halfSize = view.getSize() / 2.0f;

    int leftX = std::floor((center.x - halfSize.x) / CELL_SIZE);
    int rightX = std::ceil((center.x + halfSize.x) / CELL_SIZE);
    int topY = std::floor((center.y - halfSize.x) / CELL_SIZE);
    int bottomY = std::ceil((center.y + halfSize.y) / CELL_SIZE);

    for (int y = topY; y < bottomY; y++)
    {
        for (int x = leftX; x < rightX; x++)
        {
            if (m_field.At(x, y))
            {
                m_cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                m_target.draw(m_cell);
            }
        }
    }

    // We have to draw multiple copies of the grid if more than one copies of the field are present on screen
    int gridSizeX = CELL_SIZE * m_field.GetSize().x;
    int gridSizeY = CELL_SIZE * m_field.GetSize().y;
    int gridLeft = std::floor((center.x - halfSize.x) / gridSizeX);
    int gridRight = std::ceil((center.x + halfSize.x) / gridSizeX);
    int gridTop = std::floor((center.y - halfSize.x) / gridSizeY);
    int gridBottom = std::ceil((center.y + halfSize.y) / gridSizeY);
    for (int y = gridTop; y < gridBottom; y++)
    {
        for (int x = gridLeft; x < gridRight; x++)
        {
            sf::Transform gridTransform{sf::Transform::Identity};
            gridTransform.translate(x * CELL_SIZE * m_field.GetSize().x,
                                    y * CELL_SIZE * m_field.GetSize().y);

            sf::RenderStates states;
            states.transform = gridTransform;
            m_target.draw(&m_gridVertices[0], m_gridVertices.size(), sf::Lines, states);
        }
    }
}

void FieldRenderer::CreateGrid()
{
    m_gridVertices.reserve(2 * (m_field.GetSize().x + m_field.GetSize().y + 2));
    float left = 0;
    float top = 0;
    float right = m_field.GetSize().x * CELL_SIZE;
    float bottom = m_field.GetSize().y * CELL_SIZE;

    for (int y = 0; y <= m_field.GetSize().y; y++)
    {
        m_gridVertices.emplace_back(sf::Vertex(sf::Vector2f(left, y * CELL_SIZE),
                                               sf::Color::Green));
        m_gridVertices.emplace_back(sf::Vertex(sf::Vector2f(right, y * CELL_SIZE), sf::Color::Green));
    }

    for (int x = 0; x <= m_field.GetSize().x; x++)
    {
        m_gridVertices.emplace_back(sf::Vertex(sf::Vector2f(x * CELL_SIZE, top), sf::Color::Green));
        m_gridVertices.emplace_back(sf::Vertex(sf::Vector2f(x * CELL_SIZE, bottom), sf::Color::Green));
    }
}
