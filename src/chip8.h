#pragma once
#include "SDL2/SDL.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

class chip8 {
	
	public:
		chip8();
		~chip8();
		
		bool drawFlag;

		std::string emulateCycle();
		bool load(const char*);		
		void init();
		
		Uint32 *pixels;
		unsigned char  key[16];	
		bool sound;
		bool autoOpcode = true;

		unsigned short pc;				// Program counter
		unsigned short opcode;			// Current opcode
		unsigned short I;				// Index register
		unsigned short sp;				// Stack pointer
		
		unsigned char  V[16];			// V-regs (V0-VF)
		unsigned short stack[16];		// Stack (16 levels)
		unsigned char  memory[4096];	// Memory (size = 4k)		
				
		unsigned char  delay_timer;		// Delay timer
		unsigned char  sound_timer;		// Sound timer
};