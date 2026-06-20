#include "machine/scratchpad.hpp"

machine::Scratchpad::Scratchpad():_data({0}){}

void machine::Scratchpad::put(uint8_t index, uint32_t value) {
    this->_data[index] = value;
}

uint32_t machine::Scratchpad::get(uint8_t index) {
    return this->_data[index];
}
