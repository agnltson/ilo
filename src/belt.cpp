#include "machine/belt.hpp"

using namespace machine;

belt::Belt::Belt():_data({0}), _head(0){}

void belt::Belt::push(int32_t value) {
    this->_head = (this->_head+1) % BELT_SIZE;
    this->_data[this->_head] = value;
}

int32_t belt::Belt::get(uint8_t index) {
    return this->_data[(this->_head + BELT_SIZE - index) % BELT_SIZE];
}
