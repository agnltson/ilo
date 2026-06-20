#pragma once

#include <cstdint>

#define BELT_SIZE 32

class Belt {
    public:
        Belt();
        void push(int32_t);
        int32_t get(uint8_t) const;
    private:
        int32_t _data[BELT_SIZE];
        uint8_t _head;
};
