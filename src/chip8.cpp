#include <iostream>
#include <cstdint>
#include <bitset>
#include <stdlib.h>

#include "chip8.h"

using namespace std;

void Chip8::loadFonts(){
    for (int i = 0; i < 80; i++){
        Chip8::memory.write_resv(0x50 + i, Chip8::fontset[i]);
    }
}

uint16_t Chip8::fetch(){
    uint8_t hi = Chip8::memory.read(Chip8::PC);
    uint8_t low = Chip8::memory.read(Chip8::PC + 1);
    uint16_t opcode = ((uint16_t) hi << 8) | low;


    if (Chip8::PC == 4096){
        Chip8::PC = 0;
    }else{
        Chip8::PC += 2;
    }

    return opcode;
}

void Chip8::half_cycle(uint16_t opcode){
    uint8_t oprand = (opcode >> 12) & 0xF;

    if (oprand == 0x6){
        Chip8::V[((opcode >> 8) & 0xF)] = (opcode & 0xFF); // LD Vx, byte
    } else if (oprand == 0x8){ // 0x8 series
        if ((opcode & 0x0f) == 0x0) Chip8::V[((opcode >> 8) & 0xF)] = Chip8::V[((opcode >> 4) & 0xF)]; // LD Vx, Vy
        else if ((opcode & 0x0f) == 0x1) Chip8::V[((opcode >> 8) & 0xF)] |= Chip8::V[((opcode >> 4) & 0xF)];
        else if ((opcode & 0x0f) == 0x2) Chip8::V[((opcode >> 8) & 0xF)] &= Chip8::V[((opcode >> 4) & 0xF)];
        else if ((opcode & 0x0f) == 0x3) Chip8::V[((opcode >> 8) & 0xF)] ^= Chip8::V[((opcode >> 4) & 0xF)];
        else if ((opcode & 0x0f) == 0x4){
            uint16_t sum = Chip8::V[((opcode >> 8) & 0xF)] + Chip8::V[((opcode >> 4) & 0xF)];
            Chip8::V[((opcode >> 8) & 0xF)] = sum & 0xff;
            Chip8::V[0xf] = sum > 0xff;
        }else if ((opcode & 0x0f) == 0x5){
            uint8_t flag = Chip8::V[((opcode >> 8) & 0xF)] >= Chip8::V[((opcode >> 4) & 0xF)];
            Chip8::V[((opcode >> 8) & 0xF)] = Chip8::V[((opcode >> 8) & 0xF)] - Chip8::V[((opcode >> 4) & 0xF)];
            Chip8::V[0xf] = flag;
        }else if ((opcode & 0x0f) == 0x6){
            uint8_t flag = Chip8::V[((opcode >> 8) & 0xF)] & 1;
            Chip8::V[((opcode >> 8) & 0xF)] >>= 1;
            Chip8::V[0xf] = flag;
        }else if ((opcode & 0x0f) == 0x7){
            uint8_t flag = Chip8::V[((opcode >> 4) & 0xF)]  >= Chip8::V[((opcode >> 8) & 0xF)];
            Chip8::V[((opcode >> 8) & 0xF)] = Chip8::V[((opcode >> 4) & 0xF)] - Chip8::V[((opcode >> 8) & 0xF)];
            Chip8::V[0xf] = flag;
        } else if ((opcode & 0x0f) == 0xE){
            uint8_t flag = (Chip8::V[((opcode >> 8) & 0xF)] >> 7) & 1;
            Chip8::V[((opcode >> 8) & 0xF)] <<= 1;
            Chip8::V[0xf] = flag;
        }

    } else if (oprand == 0xA){
        Chip8::I = opcode & 0xFFF; // LD I, addr
    } else if (oprand == 0xf){ // 0xf series
        if ((opcode & 0xFF) == 0x29){
            Chip8::I = Chip8::V[((opcode >> 8) & 0xF)] * 5; // LD F, Vx
        } else if ((opcode & 0xFF) == 0x33){
            Chip8::memory.write(Chip8::I, (Chip8::V[((opcode >> 8) & 0xF)] / 100));
            Chip8::memory.write(Chip8::I + 1, ((Chip8::V[((opcode >> 8) & 0xF)] / 10) % 10));
            Chip8::memory.write(Chip8::I + 2, (Chip8::V[((opcode >> 8) & 0xF)] % 10));
        } else if ((opcode & 0xFF) == 0x55){
            uint16_t curr_addr = Chip8::I;
            for (int i = 0; i < (((opcode >> 8) & 0xF) + 1); i++){
                Chip8::memory.write(curr_addr, Chip8::V[i]); // LD [I], Vx
                curr_addr++;
            }
        } else if ((opcode & 0xFF) == 0x65){
            uint16_t curr_addr = Chip8::I;
            for (int i = 0; i < (((opcode >> 8) & 0xF) + 1); i++){
                Chip8::V[i] = memory.read(curr_addr); // LD Vx, [i]
                curr_addr++;
            }
        } else if ((opcode & 0xFF) == 0x1E){
            Chip8::I += Chip8::V[((opcode >> 8) & 0xF)]; // add i, vx
        } else if ((opcode & 0xFF) == 0x07){
            Chip8::V[((opcode >> 8) & 0xF)] = Chip8::DT;
        } else if ((opcode & 0xFF) == 0x15){
            Chip8::DT = Chip8::V[((opcode >> 8) & 0xF)];
        } else if ((opcode & 0xFF) == 0x18){
            Chip8::ST = Chip8::V[((opcode >> 8) & 0xF)];
        } else if ((opcode & 0xFF) == 0x0A){
            bool pressed = false;
            for (int i = 0; i < 16; ++i){
                if (Chip8::keypad[i]) {Chip8::V[((opcode >> 8) & 0xF)] = i; pressed = true; break;}
            }
            if (!pressed) Chip8::PC -= 2;
        }
    } else if (oprand == 0x1) {
        Chip8::PC = (opcode & 0x0FFF); // jp nnn
    }else if (oprand == 0xB) {
        Chip8::PC = ((opcode & 0x0FFF) + Chip8::V[0]);
    } else if (oprand == 0x3) {
        if (Chip8::V[((opcode >> 8) & 0xF)] == (opcode & 0xFF)){
            Chip8::PC += 2;
        }
    } else if (oprand == 0xD){
        uint8_t x = Chip8::V[(opcode >> 8) & 0xF] % 64;
        uint8_t y = Chip8::V[(opcode >> 4) & 0xF] % 32;
        uint8_t n = opcode & 0xF;
        Chip8::V[0xF] = 0;

        for (int row = 0; row < n; row++) {
            uint8_t spriteByte = Chip8::memory.read_resv(Chip8::I + row);

            for (int col = 0; col < 8; col++){
                if (spriteByte & (0x80 >> col)){
                    int px = x + col, py = y + row;
                    if (px >= 65 || py >= 32) continue;
                    uint8_t &pixel = Chip8::gfx[py * 64 + px];
                    if (pixel) V[0xF] = 1;
                    pixel ^= 1;
                }
            }
        }
        Chip8::drawFlag = true;
    } else if (oprand == 0x00){
        if (opcode == 0x00e0){
            fill(begin(Chip8::gfx), end(Chip8::gfx), 0);
            drawFlag = true;
        } else if (opcode == 0x00ee){
            Chip8::PC = Chip8::stack.pop();
        }
    } else if (oprand == 0x7){
        Chip8::V[((opcode >> 8) & 0xF)] += (opcode & 0xFF);
    } else if (oprand == 0x2){
        Chip8::stack.push(Chip8::PC);
        Chip8::PC = opcode & 0xFFF;
    } else if (oprand == 0x4){
        if (Chip8::V[((opcode >> 8) & 0xF)] != (opcode & 0xFF)){
            Chip8::PC += 2;
        }
    } else if (oprand == 0x5){
        if (Chip8::V[((opcode >> 8) & 0xF)] == Chip8::V[((opcode >> 4) & 0xF)]){
            Chip8::PC += 2;
        }
    } else if (oprand == 0x9){
        if (Chip8::V[((opcode >> 8) & 0xF)] != Chip8::V[((opcode >> 4) & 0xF)]){
            Chip8::PC += 2;
        }
    } else if (oprand == 0xE){
        if ((opcode & 0xFF) == 0x9E){
            if (Chip8::keypad[Chip8::V[((opcode >> 8) & 0xF)]]) Chip8::PC += 2;
        } else if ((opcode & 0xFF) == 0xA1){
            if (!Chip8::keypad[Chip8::V[((opcode >> 8) & 0xF)]]) Chip8::PC += 2;
        }
    }else if (oprand == 0xC) {
        Chip8::V[((opcode >> 8) & 0xF)] = (rand() % 256) & (opcode & 0x00FF);
    }
}

void Chip8::cycle(){
    uint16_t opcode = Chip8::fetch();
    //cout << static_cast<int>(Chip8::stack.debug()) << endl;
    Chip8::half_cycle(opcode);
}