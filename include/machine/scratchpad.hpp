#pragma once

#include <cstdint>

#define SCRATCHPAD_SIZE 32

namespace machine::scratchpad {
    class Scratchpad {
        public:
            Scratchpad();
            void put(uint8_t, uint32_t);
            uint32_t get(uint8_t);
        private:
            int32_t _data[SCRATCHPAD_SIZE];
    };
}
