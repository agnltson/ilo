#include <iostream>

#include "machine.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " path/to/binary" << std::endl;
        return 0;
    }

    Machine machine;
    machine.load_program(argv[1]);
    machine.run();
    return 0;
}
