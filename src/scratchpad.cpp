#include "machine/scratchpad.hpp"

using namespace machine;

scratchpad::Scratchpad::Scratchpad():_data({0}){}

void scratchpad::Scratchpad::put(uint8_t index, uint32_t value) {
    this->_data[index] = value;
}

uint32_t scratchpad::Scratchpad::get(uint8_t index) {
    return this->_data[index];
}
