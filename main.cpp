#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <thread>
#include "solver.hpp"
#include "heatmap.hpp"

int main(int argc, char *argv[])
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Visualization");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    Heatmap heatmap;
    heatmap.FillScreen(window);
    TaskParams params = {};
    ThreadCtl ctl = {false, false, 0};

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.setTitle("Visualisation, FPS: " + std::to_string(1.0 / deltaClock.getElapsedTime().asSeconds()));
        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Configuration");
        {
            ImGui::SliderInt("Threads", reinterpret_cast<int *>(&params.threads), 1, 16);
            ImGui::DragInt("Field width", reinterpret_cast<int *>(&params.width), 1.0f, 10, 1000);
            ImGui::DragInt("Field height", reinterpret_cast<int *>(&params.height), 1.0f, 10, 1000);

            ImGui::Text("Method:");
            if (ImGui::RadioButton("Jacobi", params.method == Method::Jacobi))
            {
                params.method = Method::Jacobi;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Seidel", params.method == Method::Seidel))
            {
                params.method = Method::Seidel;
            }

            ImGui::DragFloat("Left", &params.left, 1.0f, -100, 500);
            ImGui::DragFloat("Right", &params.right, 1.0f, -100, 500);
            ImGui::DragFloat("Top", &params.top, 1.0f, -100, 500);
            ImGui::DragFloat("Bottom", &params.bottom, 1.0f, -100, 500);
            ImGui::InputFloat("Epsilon", &params.eps, 0.0f, 0.0f, "%e");
            {
                std::lock_guard<std::mutex> ctlGuard{ctl.mutex};
                if (!ctl.running && ImGui::Button("Start"))
                {
                    ctl.running = true;
                    ctl.iteration = 0;
                    heatmap.Resize(params.width, params.height);
                    heatmap.FillScreen(window);
                    std::thread thread{[&params, &heatmap, &ctl]()
                                       {
                                           Solver s{params, heatmap, ctl};
                                           s.Solve();
                                       }};
                    thread.detach();
                }
                if (ctl.running && ImGui::Button("Stop"))
                {
                    ctl.stop = true;
                }
            }
            ImGui::Value("Iteration", ctl.iteration);
        }
        ImGui::End(); // end window

        window.clear(sf::Color::Cyan);
        heatmap.UpdateTexture();
        heatmap.Render(window);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}

