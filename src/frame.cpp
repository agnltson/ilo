#include "machine/frame.hpp"

Frame::Frame(uint32_t return_pc):
    belt(new Belt),
    scratchpad(new Scratchpad),
    _return_pc(return_pc) {}

Frame::Frame(uint32_t return_pc, const Belt* caller_belt):
    belt(new Belt(caller_belt)),
    scratchpad(new Scratchpad),
    _return_pc(return_pc) {}
