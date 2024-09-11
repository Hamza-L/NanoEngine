#include <cstdlib>
#include <iostream>

#include "NanoEngine.hpp"

int main(int argc, char *argv[]) {
    NanoEngine engine;
    try {
        engine.Init();
        engine.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
