#pragma once

#include <cstdint>

#define SCRATCHPAD_SIZE 32

class Scratchpad {
public:
    Scratchpad();
    void put(uint8_t, int32_t);
    int32_t get(uint8_t) const;

private:
    int32_t _data[SCRATCHPAD_SIZE];
#ifdef DBG
public:
    void debug() const;
#endif // DBG
};
