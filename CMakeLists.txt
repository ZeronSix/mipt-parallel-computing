cmake_minimum_required(VERSION 3.14)
project(game_of_life)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS window graphics REQUIRED)
find_package(OpenMP)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")

add_executable(game_of_life main.cpp Game.cpp Game.h Field.cpp Field.h FieldRenderer.cpp FieldRenderer.h)
target_link_libraries(game_of_life sfml-window sfml-graphics OpenMP::OpenMP_CXX)
