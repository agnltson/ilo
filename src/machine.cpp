#include <fstream>
#include <cstdint>

#include "machine.hpp"

#define MAGIC_CODE 0xD12EA2E2

Machine::Machine():_stack_frame(), _ram({0}), _running(false) {}

Machine::~Machine() {}

uint32_t read_u32_le(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<uint32_t>(bytes[0])
         | static_cast<uint32_t>(bytes[1]) << 8
         | static_cast<uint32_t>(bytes[2]) << 16
         | static_cast<uint32_t>(bytes[3]) << 24;
}

int check_validity(std::ifstream& file) {
    if (file.peek() != EOF) {
        uint32_t magic_code = read_u32_le(file);
        if (magic_code == MAGIC_CODE) {
            return 0;
        }
    }
    return 1;
}

int Machine::load_program(std::string binary_path) {
    std::ifstream file(binary_path, std::ios::binary);

    size_t i = 0;
    int err = check_validity(file);
    if (err) {
        return err;
    }
    while (file.peek() != EOF) {
        uint32_t value = read_u32_le(file);
        if (file) {
            this->_ram[i] = value;
            i++;
        }
    }
    return 0;
}

void Machine::run() {
    this->_running = true;
    this->_stack_frame.push_back(new Frame(0));

#ifdef DBG
    this->debug();
#endif // DBG
    while (this->_running) {
        this->process_instruction(this->_ram[this->_pc]);
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
#include <iostream>

void Machine::debug() const {
    std::cout << "#### DEBUG ###" << std::endl;
    size_t count = 0;
    for (Frame* frame: this->_stack_frame) {
        std::cout << "--- Frame " << count << " ---" << std::endl;
        frame->debug();
        count++;
    }
}
#endif // DBG

void Machine::process_instruction(uint32_t instruction) {
    switch (instruction & 0x1F) {
        default:
            break;
    }
    (void)instruction;
}

