#pragma once

#include <iostream>
#include <cstdint>
#include <array>

class Stack{
    public:
        uint8_t SP = 0x00;
        void push(uint16_t val);
        uint16_t pop();
        uint8_t debug();
    private:
        std::array<uint8_t, 32> stack{};
};
