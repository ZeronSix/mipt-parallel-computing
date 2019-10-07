#include <getopt.h>
#include <iostream>
#include <omp.h>
#include "Game.h"

constexpr int DEFAULT_NUM_THREADS{1};
constexpr const char *DEFAULT_START_FILE{""};

int main(int argc, char *argv[])
{
    std::string startFileName{DEFAULT_START_FILE};
    int numThreads = DEFAULT_NUM_THREADS;

    int opt = 0;
    while ((opt = getopt(argc, argv, "f:n:")) != -1)
    {
        switch (opt)
        {
            case 'f':
                startFileName = std::string{optarg};
                break;
            case 'n':
                numThreads = static_cast<int>(std::strtol(optarg, nullptr, 10));
                break;
            case '?':
                std::cerr << "Usage: game_of_life -n numThreads -f startFileName" << std::endl;
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    omp_set_dynamic(0);
    omp_set_num_threads(numThreads);
    Game game{1280, 720, startFileName, numThreads};
    game.Run();

    return 0;
}

