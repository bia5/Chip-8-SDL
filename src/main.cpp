#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_net.h"
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "Network.h"
#include "SplitString.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

chip8* 			myChip8 = new chip8();
SDL_Window*		window;
const Uint8*	state;
Network			network = Network();
std::string		IP = "localhost";
bool 			isGoingOnline = false;
SplitString		ss;

void keyboard(bool);
int main(int argc, char* args[]) {
	if (argc < 2) {
        std::cout << "Usage: chip8 <ROM file>" << std::endl;
        return 1;
    }
	for (int i = 0; i < argc; ++i){
		if(strcmp(args[i], "-s") == 0){
			std::cout << "Set to server!" << std::endl;
			network.setIsServer(true);
			isGoingOnline = true;
		}
		if(strcmp(args[i], "-c") == 0){
			if(argc > i+1)
				IP = args[i+1];
			else
				std::cout << "Connecting to default IP!" << std::endl;
			network.serverName = IP;
			isGoingOnline = true;
			myChip8->autoOpcode = false;
		}
	}

	SDL_Renderer*		renderer;
	SDL_Texture*		texture;
	SDL_Event       	event;
	SDL_AudioSpec 		wavSpec;
	SDL_AudioDeviceID 	deviceId;
	Uint32 				wavLength;
	Uint8*				wavBuffer;
	
	bool quit = false;
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	SDLNet_Init();
	
	if(isGoingOnline){
		network.init(network.getIsServer());
		if(!network.getIsServer()){
			network.sendMessage(network.MESSAGE_JOIN, network.getIP());
		}
	}
	
	SDL_LoadWAV("beep.wav", &wavSpec, &wavBuffer, &wavLength);

	//if(network.getIsServer())
		window = SDL_CreateWindow("Chip 8 w/ SDL2", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED); 
	//else
	//	window = SDL_CreateWindow("Chip 8 w/ SDL2", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); 
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64 , 32);
	deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
	
	myChip8->pixels = new Uint32[64 * 32];
	std::fill_n(myChip8->pixels, (64 * 32), 0);
	
	if (!myChip8->load(args[1]))
        return 2;
	
	if(network.getIsServer()){
		std::cout << "Press Any Key When All Players Have Joined" << std::endl;
		system("pause");
	}
	
	while (true) {
		while(SDL_PollEvent(&event)) {
			switch( event.type ) {
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					state = SDL_GetKeyboardState(NULL);
					keyboard(true);
					break;
				case SDL_KEYUP:
					keyboard(false);
					break;
				default:
					break;
			}
		}
		
		if(isGoingOnline){
			if(network.getIsServer()){
				//Grab Client Inputs
				std::vector<ClientMessage> msgs = network.update();
				
				//Handle Client Inputs
				for(ClientMessage cm : msgs){
					std::vector<std::string> values = ss.split(cm.message, ",");
					for(std::string strang : values){
						std::vector<std::string> valuess = ss.split(strang, ":");
						if(valuess[0] == "k"){	//Key Update
							myChip8->key[std::stoi(valuess[1])] = std::stoi(valuess[2]);
						}
					}
				}
				
				//Tick
				std::string out = myChip8->emulateCycle();
				
				//Send Screen To Clients
				if(out != "null"){
					for(Client* client : network.clients){
						network.sendMessage(out, client->ip);
					}
				}
			} else {
				std::vector<ClientMessage> msgs = network.update();
				for(ClientMessage cm : msgs){
					std::vector<std::string> values = ss.split(cm.message, ",");
					for(std::string strang : values){
						std::vector<std::string> valuess = ss.split(strang, ":");
						if(valuess[0] == "p"){	//Set Pixel
							myChip8->pixels[std::stoi(valuess[1])] ^= 0xFFFFFFFF;
						}
						if(valuess[0] == "o"){ //Set opcode
							myChip8->opcode = std::stoi(valuess[1]);
							myChip8->emulateCycle();
						}
					}
				}
			}
		} else {
			std::string out = myChip8->emulateCycle();
		}
		
		//Render
		if(myChip8->drawFlag) {
			SDL_UpdateTexture(texture, NULL, myChip8->pixels, 64 * sizeof(Uint32));
		}

		if(myChip8->sound) {
			int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
			SDL_PauseAudioDevice(deviceId, 0);
			myChip8->sound = false;
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(2);
	}
	
	SDL_CloseAudioDevice(deviceId);
	SDL_FreeWAV(wavBuffer);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window); 
	SDL_Quit(); 

	return 0;   
}

void keyboard(bool press) {
	unsigned char  _key[16];
	std::copy(std::begin(myChip8->key), std::end(myChip8->key), std::begin(_key));
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
	} else {
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
	
	if(!network.getIsServer() && isGoingOnline){
		std::string oua = "";
		for(int i = 0; i<16; i++){
			if(myChip8->key[i] != _key[i])
				oua = oua + ",k:" + std::to_string(i) + ":" + std::to_string(myChip8->key[i]);
		}
		if(oua != "")
			network.sendMessage(oua, network.getIP());
	}
}