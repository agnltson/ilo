#pragma once

#include <cstdint>

#include "machine/belt.hpp"
#include "machine/scratchpad.hpp"

class Frame {
public:
    Frame(uint32_t);
    Frame(uint32_t, const Belt*);
    ~Frame();

    Belt* belt;
    Scratchpad* scratchpad;
    uint32_t return_pc;

private:
#ifdef DBG
public:
    void debug() const;
#endif // DBG
};
