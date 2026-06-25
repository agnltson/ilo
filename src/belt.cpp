#include <cstring>

#include "machine/belt.hpp"

Belt::Belt() : _data(0), _head(0) {
}

Belt::Belt(const Belt* caller_belt) : _head(caller_belt->_head) {
    memcpy(_data, caller_belt->_data, sizeof(int32_t) * BELT_SIZE);
}

void Belt::push(int32_t value) {
    this->_head = (this->_head + 1) % BELT_SIZE;
    this->_data[this->_head] = value;
}

int32_t Belt::get(uint8_t index) const {
    return this->_data[(this->_head + BELT_SIZE - index) % BELT_SIZE];
}

#ifdef DBG
#include <iostream>

void Belt::debug() const {
    std::cout << "Belt: ";
    for (uint8_t i = 0; i < BELT_SIZE; ++i) {
        std::cout << this->get(i) << " ";
    }
    std::cout << std::endl;
}
#endif // DBG
