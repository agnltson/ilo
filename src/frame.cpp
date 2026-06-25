#include "machine/frame.hpp"

Frame::Frame(uint32_t pc) : belt(new Belt), scratchpad(new Scratchpad), return_pc(pc) {
}

Frame::Frame(uint32_t pc, const Belt* caller_belt)
    : belt(new Belt(caller_belt)), scratchpad(new Scratchpad), return_pc(pc) {
}

#ifdef DBG
#include <iostream>

void Frame::debug() const {
    std::cout << "return_pc: " << this->return_pc << std::endl;
    this->belt->debug();
    this->scratchpad->debug();
}
#endif // DBG
