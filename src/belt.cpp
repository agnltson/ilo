#include "machine/belt.hpp"

belt::Belt::Belt():_data({0}), _head(0){}

void belt::Belt::push(uint32_t value) {
    this->_head = (this->_head+1) % BELT_SIZE;
    this->_data[this->_head] = value;
}

uint32_t belt::Belt::get(uint8_t index) {
    return this->_data[(this->_head + BELT_SIZE - index) % BELT_SIZE];
}
