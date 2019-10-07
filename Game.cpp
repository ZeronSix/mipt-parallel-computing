//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#include <iostream>
#include "Game.h"
#include "Field.h"

Game::Game(unsigned int screenWidth, unsigned int screenHeight, const std::string &fileName, int threads):
    m_window{sf::VideoMode{screenWidth, screenHeight}, "John Conway's Game of Life"},
    m_field{Field::LoadFromFile(fileName)},
    m_renderer{m_field, m_window},
    m_timer{sf::seconds(0)},
    m_zoom{1.0f}
{
}

void Game::Run()
{
    auto mousePos = sf::Mouse::getPosition();
    auto view = m_window.getDefaultView();

    while (m_window.isOpen())
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.wheel != sf::Mouse::VerticalWheel) continue;

                float newZoom = m_zoom - event.mouseWheelScroll.delta * ZOOM_STEP;
                newZoom = std::min(MAX_ZOOM, std::max(MIN_ZOOM, newZoom));
                view.zoom(newZoom / m_zoom);
                m_zoom = newZoom;
            }
            else if (event.type == sf::Event::Resized)
            {
                // update the view to the new size of the window
                view.setSize(event.size.width, event.size.height);
                view.zoom(m_zoom);
            }
        }

        auto newMousePos = sf::Mouse::getPosition();
        auto mouseDelta = newMousePos - mousePos;
        mousePos = newMousePos;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
        {
            view.move(-mouseDelta.x * MOUSE_DRAG_SENSITIVITY * m_zoom,
                      -mouseDelta.y * MOUSE_DRAG_SENSITIVITY * m_zoom);
        }
        m_window.setView(view);

        m_window.clear(sf::Color::White);
        if (!m_field.IsGameOver())
        {
            m_field.Update();
        }
        m_renderer.Render();
        m_window.display();

        sf::Time newTime = m_clock.getElapsedTime();
        UpdateWindowTitle((newTime - m_timer).asSeconds());
        m_timer = newTime;
    }
}
void Game::UpdateWindowTitle(float dt)
{
    if (dt == 0) return;
    std::string title = m_field.IsGameOver() ? "Game Over!" :
                        "FPS: " + std::to_string(1.0f / dt);
    m_window.setTitle(title);
}
