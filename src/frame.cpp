#include "machine/frame.hpp"

Frame::Frame(uint32_t return_pc):
    belt(new Belt),
    scratchpad(new Scratchpad),
    _return_pc(return_pc) {}

Frame::Frame(uint32_t return_pc, const Belt* caller_belt):
    belt(new Belt(caller_belt)),
    scratchpad(new Scratchpad),
    _return_pc(return_pc) {}

#ifdef DBG
#include <iostream>

void Frame::debug() const {
    std::cout << "return_pc: " << this->_return_pc << std::endl;
    this->belt->debug();
    this->scratchpad->debug();
}
#endif // DBG
