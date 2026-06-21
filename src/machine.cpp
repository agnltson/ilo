#include <fstream>
#include <cstdint>
#include <iostream>

#include "machine.hpp"

Machine::Machine():_stack_frame(), _ram({0}), _running(false) {}

Machine::~Machine() {}

uint32_t read_u32_le(std::ifstream& f) {
    uint8_t bytes[4];
    f.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<uint32_t>(bytes[0])
         | static_cast<uint32_t>(bytes[1]) << 8
         | static_cast<uint32_t>(bytes[2]) << 16
         | static_cast<uint32_t>(bytes[3]) << 24;
}

void Machine::load_program(std::string binary_path) {
    std::ifstream file(binary_path, std::ios::binary);

    size_t i = 0;
    while (file.peek() != EOF) {
        uint32_t value = read_u32_le(file);
        if (file) {
            this->_ram[i] = value;
            i++;
        }
    }
}

void Machine::run() {
#ifdef DBG
    this->debug();
#endif // DBG
    this->_running = true;

    while (this->_running) {
    }
}

int32_t Machine::frame_pop() {
    Frame* top_frame = this->_stack_frame.back();
    this->_stack_frame.pop_back();
    int32_t retval = top_frame->belt->get(0);
    return retval;
}

void Machine::frame_add() {
    Frame* new_frame = new Frame(this->_pc+1, this->_stack_frame.back()->belt);
    this->_stack_frame.push_back(new_frame);
}

void Machine::inc_pc() {
    ++this->_pc;
}

#ifdef DBG
void Machine::debug() {
    std::cout << "TODO" << std::endl;
}
#endif // DBG
