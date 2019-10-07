//
// Created by Vyacheslav Zhdanovskiy <zhdanovskiy@iitp.ru> on 10/3/19.
//

#ifndef FIELD_H
#define FIELD_H

#include <cstddef>
#include <vector>
#include <set>
#include <SFML/System.hpp>

class Field
{
public:
    Field(int width, int height);
    Field(Field &&other) noexcept;
    ~Field();

    const bool &At(int x, int y) const;
    sf::Vector2i GetSize() const;
    void Update();
    bool IsGameOver() const;

    static constexpr int DEFAULT_WIDTH{100};
    static constexpr int DEFAULT_HEIGHT{100};
    static Field LoadFromFile(const std::string &fileName);
private:
    bool *m_memory;
    bool *m_startMemory;
    int m_width;
    int m_height;
    bool m_isGameOver;
    int m_threads;

    using ChangesByThread = std::vector<std::vector<std::pair<int, int>>>;
    ChangesByThread m_prevChangesByThread;

    bool &At(int x, int y);
};


#endif //FIELD_H
