#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "chip8.h"
#include <unistd.h>
#include <algorithm>
#include <iostream>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

using namespace std;

chip8* 				myChip8;
SDL_Window*		window;
const Uint8*	state;

void keyboard(bool);

int main(int argc, char* args[]) { 
	if (argc != 2) {
		cout << "Usage: chip8 <ROM file>" << endl;
		return 1;
  }

	SDL_Renderer*				renderer;
	SDL_Texture*				texture;
	SDL_Event						event;
	SDL_AudioSpec				wavSpec;
	SDL_AudioDeviceID 	deviceId;
	Uint32							wavLength;
	Uint8*							wavBuffer;
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDL_LoadWAV("beep.wav", &wavSpec, &wavBuffer, &wavLength);

	window = SDL_CreateWindow("Chip 8 w/ SDL2", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); 
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

	myChip8 = new chip8();
	myChip8->pixels = new Uint32[64 * 32]; 
	fill_n(myChip8->pixels, (64 * 32), 0);
	
	if (!myChip8->load(args[1]))
        return 2;
	
	while(true) {		
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					exit(0);
					break;
			  case SDL_KEYDOWN:
					state = SDL_GetKeyboardState(NULL);
					keyboard(true);
					break;
			  case SDL_KEYUP:
					state = SDL_GetKeyboardState(NULL);
					keyboard(false);
					break;
			  default:
					break;
			}
		}
		
		myChip8->emulateCycle();

		if(myChip8->sound) {
			int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
			SDL_PauseAudioDevice(deviceId, 0);
			myChip8->sound = false;
		}
		
		if(myChip8->drawFlag) {
			SDL_UpdateTexture(texture, NULL, myChip8->pixels, 64 * sizeof(Uint32));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		SDL_Delay(15); //"vsync"
	}

	SDL_CloseAudioDevice(deviceId);
	SDL_FreeWAV(wavBuffer);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window); 
	SDL_Quit(); 
	return 0;   
}

void keyboard(bool press){	
	if(press) {
		if(state[SDL_SCANCODE_ESCAPE])
			exit(0);

		if(state[SDL_SCANCODE_1])		myChip8->key[0x1] = 1;
		if(state[SDL_SCANCODE_2])		myChip8->key[0x2] = 1;
		if(state[SDL_SCANCODE_3])		myChip8->key[0x3] = 1;
		if(state[SDL_SCANCODE_4])		myChip8->key[0xC] = 1;

		if(state[SDL_SCANCODE_Q])		myChip8->key[0x4] = 1;
		if(state[SDL_SCANCODE_W])		myChip8->key[0x5] = 1;
		if(state[SDL_SCANCODE_E])		myChip8->key[0x6] = 1;
		if(state[SDL_SCANCODE_R])		myChip8->key[0xD] = 1;

		if(state[SDL_SCANCODE_A])		myChip8->key[0x7] = 1;
		if(state[SDL_SCANCODE_S])		myChip8->key[0x8] = 1;
		if(state[SDL_SCANCODE_D])		myChip8->key[0x9] = 1;
		if(state[SDL_SCANCODE_F])		myChip8->key[0xE] = 1;

		if(state[SDL_SCANCODE_Z])		myChip8->key[0xA] = 1;
		if(state[SDL_SCANCODE_X])		myChip8->key[0x0] = 1;
		if(state[SDL_SCANCODE_C])		myChip8->key[0xB] = 1;
		if(state[SDL_SCANCODE_V])		myChip8->key[0xF] = 1;
	}
	else {
		if(!(state[SDL_SCANCODE_1]))	myChip8->key[0x1] = 0;
		if(!(state[SDL_SCANCODE_2]))	myChip8->key[0x2] = 0;
		if(!(state[SDL_SCANCODE_3]))	myChip8->key[0x3] = 0;
		if(!(state[SDL_SCANCODE_4]))	myChip8->key[0xC] = 0;

		if(!(state[SDL_SCANCODE_Q]))	myChip8->key[0x4] = 0;
		if(!(state[SDL_SCANCODE_W]))	myChip8->key[0x5] = 0;
		if(!(state[SDL_SCANCODE_E]))	myChip8->key[0x6] = 0;
		if(!(state[SDL_SCANCODE_R]))	myChip8->key[0xD] = 0;

		if(!(state[SDL_SCANCODE_A]))	myChip8->key[0x7] = 0;
		if(!(state[SDL_SCANCODE_S]))	myChip8->key[0x8] = 0;
		if(!(state[SDL_SCANCODE_D]))	myChip8->key[0x9] = 0;
		if(!(state[SDL_SCANCODE_F]))	myChip8->key[0xE] = 0;

		if(!(state[SDL_SCANCODE_Z]))	myChip8->key[0xA] = 0;
		if(!(state[SDL_SCANCODE_X]))	myChip8->key[0x0] = 0;
		if(!(state[SDL_SCANCODE_C]))	myChip8->key[0xB] = 0;
		if(!(state[SDL_SCANCODE_V]))	myChip8->key[0xF] = 0;
	}
}
