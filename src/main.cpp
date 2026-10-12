#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "mem.h"
#include "chip8.h"
#include "stack.h"

using namespace std;

Chip8 ch8;
auto last = chrono::steady_clock::now();

void audioCallback(void* userdata, Uint8* stream, int len) {
    double* phase = (double*)userdata;
    int16_t* out = (int16_t*)stream;
    for (int i = 0; i < len / 2; i++) {
        out[i] = (*phase < 0.5) ? 500 : -500; // volume
        *phase += 220.0 / 44100.0; // 440 Hz tone
        if (*phase >= 1.0) *phase -= 1.0;
    }
}

bool processInput(uint8_t* keypad){
    static const SDL_KeyCode keymap[16] = {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3,
        SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c,
        SDLK_4, SDLK_r, SDLK_f, SDLK_v
    };

    SDL_Event e;
    
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return true;
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) return true;
            for (int i = 0; i < 16; ++i)
                if (e.key.keysym.sym == keymap[i])
                    keypad[i] = (e.type == SDL_KEYDOWN);
        }
    }
    return false; // keep running
}

int main(int argc, const char* argv[]){
    // TODO: Run Code

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* win = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    double phase = 0.0;
    SDL_AudioSpec want{};
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = &phase;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);


    if (argc < 2){
        cerr << "Error: Please specify a rom file!" << endl;
    }
    else {
        ch8.memory.loadRom(argv[1]);

        ch8.loadFonts();

        srand(time(NULL));

        bool running = false;

        while (!running){
            SDL_Event e;
            cout << "\033[2J\033[H";
            //cout << static_cast<int>(ch8.PC) << endl;
            running = processInput(ch8.keypad);

            ch8.cycle();

            SDL_PauseAudioDevice(dev, ch8.ST == 0 ? 1 : 0);

            auto now = chrono::steady_clock::now();
            auto ms = chrono::duration_cast<chrono::milliseconds>(now - last).count();

            if (ms >= 16){
                if (ch8.DT > 0) ch8.DT--;
                if (ch8.ST > 0) ch8.ST--;
                last = now;
            }
            uint32_t pixels[64 * 32];

            if (ch8.drawFlag){
                for (int i = 0; i < 64 * 32; i++){
                    pixels[i] = ch8.gfx[i] ? 0xFFFFFFFF : 0xFF000000;
                }

                SDL_UpdateTexture(tex, nullptr, pixels, 64 * sizeof(uint32_t));
                SDL_RenderClear(ren);
                SDL_RenderCopy(ren, tex, nullptr, nullptr);
                SDL_RenderPresent(ren);

                ch8.drawFlag = false;
            }
            SDL_Delay(1);

        }

        SDL_DestroyRenderer(ren);
        SDL_DestroyTexture(tex);
        SDL_DestroyWindow(win);

        SDL_CloseAudioDevice(dev);

        SDL_Quit();
    }
    return 0;
}