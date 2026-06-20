#pragma once

#include <vector>

#include "machine/frame.hpp"

#define MEMORY_SIZE 65536

class Machine {
    public:
        Machine();
        ~Machine();
        void run();
        void inc_pc();
    private:
        int32_t frame_pop(); // pop the last frame and returns the value on frame.belt[0]
        void frame_add(); // push a new frame on the stack, the new frame belt is a copy of the frame under it

        int32_t _pc;
        std::vector<Frame*> _stack_frame;
        int32_t _ram[MEMORY_SIZE];
        bool _running;
};
