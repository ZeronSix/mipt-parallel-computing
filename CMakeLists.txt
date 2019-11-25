cmake_minimum_required(VERSION 3.14)
project(heat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};./cmake")
message(${CMAKE_MODULE_PATH})

find_package(SFML 2.5 COMPONENTS window graphics REQUIRED)
find_package(OpenMP)
find_package(ImGui-SFML REQUIRED)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")

add_executable(heat main.cpp solver.hpp matrix.hpp heatmap.hpp)
target_link_libraries(heat sfml-window sfml-graphics OpenMP::OpenMP_CXX ImGui-SFML::ImGui-SFML)
