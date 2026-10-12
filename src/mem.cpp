#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <array>
#include <cstdint>

#include "mem.h"

using namespace std;

uint8_t Memory::read(int addr){
    if (addr <= 0x1000){
        if (addr >= 0x200){
            return Memory::memory[addr];
        } else {
            cerr << "Cannot access restricted  memory address at: [0x" << std::hex << addr<< "]!" << endl;
            return 0;
        }
    } else {
        cerr << "Memory Overflow at: [0x" << std::hex << addr << "]!" << endl;
        return 0;
    }
}

uint8_t Memory::read_resv(int addr){
    if (addr <= 0x1000){
        return Memory::memory[addr];
    } else {
        cerr << "Memory Overflow at: [0x" << std::hex << addr << "]!" << endl;
        return 0;
    }
}


void Memory::write(int addr, int value){
    if (addr <= 0x1000){
        if (addr >= 0x200){
            Memory::memory[addr] = value;
        } else {
            cerr << "Cannot access restricted  memory address at: [0x" << std::hex << addr << "]!" << endl;
        }
    } else {
        cerr << "Memory Overflow at: [" << std::hex << addr << "]!" << endl;
    }
}

void Memory::write_resv(int addr, int value){
    if (addr <= 0x1000){
        Memory::memory[addr] = value;
    } else {
        cerr << "Memory Overflow at: [" << std::hex << addr << "]!" << endl;
    }
}

void Memory::loadRom(string filePath){
    ifstream rom(filePath, ios::binary);

    int x = Memory::prog_idx;

    char byte;
    if (rom.is_open()){
        while(rom.get(byte)){
            write(x++, static_cast<uint8_t>(byte));
        }
    } else {
        cerr << "File not found!" << endl;
    }
}