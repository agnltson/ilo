#pragma once

#include "machine/belt.hpp"
#include "machine/scratchpad.hpp"

class Frame {
    public:
        Frame();
        ~Frame();
    private:
        Belt _belt;
        Scratchpad _scratchpad;
        size_t _return_pc;
};
