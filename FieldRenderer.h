//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#ifndef FIELDRENDERER_H
#define FIELDRENDERER_H

#include <SFML/Graphics.hpp>
#include "Field.h"

class FieldRenderer
{
public:
    FieldRenderer(Field &field, sf::RenderTarget &target);
    void Render();

    static constexpr int CELL_SIZE{64};
private:
    const Field &m_field;
    std::vector<sf::Vertex> m_gridVertices;
    sf::RenderTarget &m_target;
    sf::RectangleShape m_cell;

    void CreateGrid();
};


#endif //FIELDRENDERER_H
