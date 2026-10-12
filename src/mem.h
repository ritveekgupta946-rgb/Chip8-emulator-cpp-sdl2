#pragma once

#include <iostream>
#include <cstdint>
#include <array>


class Memory{
    public:
        uint8_t read(int addr);
        void write(int addr, int value);
        void loadRom(std::string filePath);
        uint8_t read_resv(int addr);
        void write_resv(int addr, int value);
    private:
        std::array<uint8_t, 4096> memory{};
        uint16_t prog_idx = 0x200;
};