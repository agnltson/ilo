#pragma once

#include <vector>
#include <string>

#include "machine/frame.hpp"

#define MEMORY_SIZE 65536

class Machine {
    public:
        Machine();
        ~Machine();
        int load_program(std::string);
        void run();
        void inc_pc();
    private:
        int32_t frame_pop(); // pop the last frame and returns the value on frame.belt[0]
        void frame_add(); // push a new frame on the stack, the new frame belt is a copy of the frame under it
        void process_instruction(uint32_t);
        void push_belt(int32_t);
        int32_t get_belt(uint8_t);
        void put_scratchpad(uint8_t, int32_t);
        int32_t get_scratchpad(uint8_t);

        int32_t _pc;
        std::vector<Frame*> _stack_frame;
        int32_t _ram[MEMORY_SIZE];
        bool _running;

        void add(uint8_t, uint8_t);
        void sub(uint8_t, uint8_t);
        void mul(uint8_t, uint8_t);
        void div(uint8_t, uint8_t);
        void andd(uint8_t, uint8_t);
        void orr(uint8_t, uint8_t);
        void xorr(uint8_t, uint8_t);
        void sll(uint8_t, uint8_t);
        void srl(uint8_t, uint8_t);
        void sra(uint8_t, uint8_t);
        void eq(uint8_t, uint8_t);
        void lt(uint8_t, uint8_t);
        void load(uint8_t);
        void store(uint8_t, uint8_t);
        void put(uint8_t, uint8_t);
        void pick(uint8_t);
        void immh(int32_t);
        void imml(int32_t);
        void jmp(int32_t);
        void jmpif(int32_t);
        void call(int32_t);
        void ret();
        void halt();

#ifdef DBG
    public:
        void debug() const;
#endif // DBG
      };
