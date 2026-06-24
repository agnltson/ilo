#include <iostream>

#include "machine.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " path/to/binary" << std::endl;
        return 0;
    }

    Machine machine;
    auto load_res = machine.load_program(argv[1]);
    if (!load_res) {
        log(load_res.error());
        return 1;
    }
    machine.run();
    return 0;
}
