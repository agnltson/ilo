#include "machine/scratchpad.hpp"

Scratchpad::Scratchpad():_data({0}){}

void Scratchpad::put(uint8_t index, int32_t value) {
    this->_data[index] = value;
}

int32_t Scratchpad::get(uint8_t index) const {
    return this->_data[index];
}
