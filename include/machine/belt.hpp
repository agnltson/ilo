#pragma once

#include <cstdint>

#define BELT_SIZE 32

namespace machine::belt {
    class Belt {
        public:
            Belt();
            void push(int32_t);
            int32_t get(uint8_t);
        private:
            int32_t _data[BELT_SIZE];
            uint8_t _head;
    };
}
