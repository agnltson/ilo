#include "machine/scratchpad.hpp"

Scratchpad::Scratchpad():_data({0}){}

void Scratchpad::put(uint8_t index, int32_t value) {
    this->_data[index] = value;
}

int32_t Scratchpad::get(uint8_t index) const {
    return this->_data[index];
}

#ifdef DBG
#include <iostream>

void Scratchpad::debug() const {
    std::cout << "Scratchpad:";
    for (int i = 0; i < SCRATCHPAD_SIZE; i++) {
        if (!(i % 8)) {
            std::cout << std::endl;
        }
        std::cout << this->_data[i] << " ";
    }
}
#endif // DBG
