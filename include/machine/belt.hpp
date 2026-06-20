#pragma once

#include <cstdint>

#define BELT_SIZE 32

namespace belt {
    class Belt {
        public:
            Belt();
            void push(uint32_t);
            uint32_t get(uint8_t);
        private:
            int32_t _data[BELT_SIZE];
            uint8_t _head;
    };
}
