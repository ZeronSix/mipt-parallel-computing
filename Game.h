//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "FieldRenderer.h"

class Game
{
public:
    Game(unsigned int screenWidth, unsigned int screenHeight, const std::string &fileName, int threads);
    void Run();
private:
    sf::RenderWindow m_window;
    Field m_field;
    FieldRenderer m_renderer;
    sf::Clock m_clock;
    sf::Time m_timer;
    float m_zoom;

    void UpdateWindowTitle(float dt);

    static constexpr float MOUSE_DRAG_SENSITIVITY{0.5f};
    static constexpr float MIN_ZOOM{0.5f};
    static constexpr float MAX_ZOOM{4.f};
    static constexpr float ZOOM_STEP{0.25f};
};


#endif //GAME_H
