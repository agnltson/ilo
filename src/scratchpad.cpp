#include "machine/scratchpad.hpp"

using namespace machine;

scratchpad::Scratchpad::Scratchpad():_data({0}){}

void scratchpad::Scratchpad::put(uint8_t index, int32_t value) {
    this->_data[index] = value;
}

int32_t scratchpad::Scratchpad::get(uint8_t index) {
    return this->_data[index];
}
