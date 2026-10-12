#include <iostream>
#include <cstdint>

#include "stack.h"

using namespace std;

void Stack::push(uint16_t val){
    uint8_t hi = (val >> 8);
    uint8_t lo = val & 0x00ff;

    Stack::stack[Stack::SP] = hi;
    Stack::stack[Stack::SP + 1] = lo;

    //cout << static_cast<int>(Stack::debug()) << endl;

    Stack::SP += 2;
}

uint16_t Stack::pop(){
    Stack::SP -= 2;
    uint16_t res = (static_cast<uint16_t>(Stack::stack[Stack::SP]) << 8) | Stack::stack[Stack::SP + 1];
    Stack::stack[Stack::SP] = 0x00;
    Stack::stack[Stack::SP + 1] = 0x00;

    return res;
}

uint8_t Stack::debug(){
    return Stack::stack[Stack::SP];
}