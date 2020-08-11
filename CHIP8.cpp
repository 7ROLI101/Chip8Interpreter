#include "CHIP8.h"
#include <iostream>
#include <fstream>
using namespace std;

//fontset that the CHIP8 is using(stored in the interpreter anywhere between address 0x000 to 0x1FF)
//out of the 8 bits given for each of the characters, we use the most significant 4 bits and ignore the least significant bits  
unsigned char fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // shows 0
  0x20, 0x60, 0x20, 0x20, 0x70, // shows 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // shows 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // shows 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // shows 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // shows 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // shows 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // shows 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // shows 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // shows 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // shows A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // shows B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // shows C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // shows D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // shows E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // shows F
};

//initialize the CHIP8 system(clears all registers, memory, and the screen)
void CHIP8system::init()
{
	I = NULL;
	pc = 0x200;//ROMs will take memory from 0x200 to 0xFFF, so instead of setting the starting point at 0x000, we start at 0x200
	//clearing everything else by setting to NULL
	opcode = NULL;
	delay = NULL;
	sound = NULL;
	sp = NULL;
	//loading the fontset(stored in the interpreter from address 0x000 to 0x1FF)
	for (int i = 0; i < 80; i++)
	{
		memory[i] = fontset[i];
	}
	//clearing out the rest of the memory
	for (int i = 512; i < 4096; i++)
	{
		memory[i] = 0;
	}
}

//loads the game into memory
//WORRY ABOUT THIS LATER
bool CHIP8system::loadgame(const char* gamefile)
{
	init();
	//using fstream to read from the file
	ifstream file;
	//opening up the ROM file 
	file.open(gamefile, fstream::in | fstream::binary);//read the file in binary
	int totalspace;
	if (file)//if a file is given 
	{
		file.seekg(0, file.end);//sets the position of the next character to be taken from the input file(going from 0(aka the beginning) to the end)
		totalspace = static_cast<int>(file.tellg());//telling us how much space the file takes up
		file.seekg(0, file.beg);////resetting the position back to the beginning
	}
	else 
	{
		std::cout << "No file given, time to exit." << std::endl;
	}
	 char *buffer = new char(totalspace);
	 file.read(buffer, totalspace);
	//loading the ROM into memory
	//checking to see if the file is small enough for the CHIP8 to hold
	 if((4096 - 512) > totalspace)
	 {
		 for (int i = 0; i < totalspace; i++)
		{
		 memory[i + 512] = buffer[i];
		}
	 }
	 else
	 {
		 cout << "The file is too big\n";
	 }
	 //finishing loading the file to memory
	 file.close();
	 delete buffer;

	 return true;
}

//emulates one cycle
//IMPORTANT, DO THIS FIRST
void CHIP8system::emulatecycle()
{
	cout << pc << "\n";
	drawflag = false;
	//getting the opcode
	/*Since the opcodes are stored sequentially in 2 bytes, you would get the address of the first two bytes, and then shift it by 2 bytes and then use the OR operation
	and then add the next two bytes with the first two to get the full opcode*/
	opcode = memory[pc] << 8 | memory[pc + 1];
	int q = opcode & 0xF000;
	//find out what the opcode actually does
	switch (opcode & 0xF000)//takes the first byte of the opcode and figure out what operation is needed from that first byte.
	//In the case of opcodes having the same first byte, you would need to have a switch statement with the LAST bytes to find out what you need to do with the opcode
	{
	case 0x0000://Decoding the opcodes starting with a 0
	{
		switch (opcode & 0x000F)//decoding the last byte, since there are 2 opcodes with the same first byte
		{
		case 0x0000://decoding 0x00E0: should clear screen
		{
			drawflag = true;
			for (int i = 0; i < 2048; i++)
			{
				display[i] = 0;
			}
			pc = pc + 2;//moving the program counter to the next opcode
			break;
		}
		case 0x000E://decoding 0x00EE:Should return from a subroutine
		{
			--sp;//decrement the stack pointer to reduce the number of stack levels
			pc = stack[sp];//returns from the subroutine with the address that was saved in the stack
			pc = pc + 2;//moving the program counter to the next opcode
			break;
		}
		}
		break;
	}
	case 0x1000://decoding 1NNN: should jump to address NNN
	{
		pc = opcode & 0x0FFF;//jumping to address 0X0NNN
		break;
	}
	case 0x2000://decoding 2NNN: should call a subroutine at NNN
	{
		stack[sp] = pc;//storing the pc counter into the stack
		++sp;//preincrementing the stack;
		pc = opcode & 0x0FFF;//going to address NNN to call the subroutine 
		//since a subroutine is being called at address NNN, the pc doesn't need to be incremented
		break;
	}
	case 0x3000://decoding 3XNNs:skips the next instruction if the next instruction Vx is equal to NN(usually the next instruction is a jump to skip a code block)
	{
		if (reg[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))//you get the register you would want by using the opcode and 0x0F00 and then AND
			//after that, you would get 8 0s, so you would have to shift the number by 8 to get the actual number you need
		{
			pc = pc + 4;//in order to skip the next instruction completely, you need to move by 4 bytes

		}
		else
		{
			pc = pc + 2;//just move on to the next instruction
		}
		break;
	}
	case 0x4000://decoding 4XNN:skips the next instruction if Vx is not equal to NN(similar as the opcode for 3XNN)
	{
		if (reg[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))//you get the register you would want by using the opcode and 0x0F00 and then AND
			//after that, you would get 8 0s, so you would have to shift the number by 8 to get the actual number you need
		{
			pc = pc + 4;//in order to skip the next instruction completely, you need to move by 4 bytes

		}
		else
		{
			pc = pc + 2;//just move on to the next instruction
		}
		break;
	}
	case 0x5000://decoding 5XY0: skips the next instruction if Vx equals Vy(similar to the 3XNN and the 4XNN)
	{
		if (reg[(opcode & 0x0F00) >> 8] == reg[(opcode & 0x00F0) >> 8])//you get the register you would want by using the opcode and 0x0F00 and then AND
			//after that, you would get 8 0s, so you would have to shift the number by 8 to get the actual number you need
		{
			pc = pc + 4;//in order to skip the next instruction completely, you need to move by 4 bytes

		}
		else
		{
			pc = pc + 2;//just move on to the next instruction
		}
		break;
	}
	case 0x6000://decoding 6XNN: setting Vx to NN
	{
		reg[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);//setting Vx to NN
		pc = pc + 2;
		break;
	}
	case 0x7000://decoding 7XNN:adding NN to Vx
	{
		reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x0F00) >> 8] + (opcode & 0x00FF);//adding NN to Vx
		pc = pc + 2;
		break;
	}
	case 0x8000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x8XY0: Sets VX to the value of VY
			reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
			reg[(opcode & 0x0F00) >> 8] |= reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
			reg[(opcode & 0x0F00) >> 8] &= reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
			reg[(opcode & 0x0F00) >> 8] ^= reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
			if (reg[(opcode & 0x00F0) >> 4] > (0xFF - reg[(opcode & 0x0F00) >> 8]))
				reg[0xF] = 1; //carry
			else
				reg[0xF] = 0;
			reg[(opcode & 0x0F00) >> 8] += reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (reg[(opcode & 0x00F0) >> 4] > reg[(opcode & 0x0F00) >> 8])
				reg[0xF] = 0; // there is a borrow
			else
				reg[0xF] = 1;
			reg[(opcode & 0x0F00) >> 8] -= reg[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
			reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0x1;
			reg[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (reg[(opcode & 0x0F00) >> 8] > reg[(opcode & 0x00F0) >> 4])	// VY-VX
				reg[0xF] = 0; // there is a borrow
			else
				reg[0xF] = 1;
			reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4] - reg[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
			reg[0xF] = reg[(opcode & 0x0F00) >> 8] >> 7;
			reg[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;
		}
		break;	}
	case 0x9000://decoding 9XY0: skips the next instruction if Vx is not equal to Vy(opposite to 5XY0)
	{
		if (reg[(opcode & 0x0F00) >> 8] != reg[(opcode & 0x00F0) >> 8])//you get the register you would want by using the opcode and 0x0F00 and then AND
			//after that, you would get 8 0s, so you would have to shift the number by 8 to get the actual number you need
		{
			pc = pc + 4;//in order to skip the next instruction completely, you need to move by 4 bytes

		}
		else
		{
			pc = pc + 2;//just move on to the next instruction
		}
		break;
	}
	case 0xA000://decoding ANNN: sets I to address NNN
	{
		I = opcode & 0x0FFF;//setting I to address NNN
		pc = pc + 2;
		break;
	}
	case 0xB000://decoding BNNN: jumps to the address NNN+register0
	{
		pc = (opcode & 0X0FFF) + reg[0];//jumping to address NNN and V[0]
		pc = pc + 2;//moving to the next instruction 
		break;
	}
	case 0xC000://decoding CXNN:setting Vx to the result of a bitwise and operation on a random number and NN
	{
		reg[(opcode & 0x0F00) >> 8] = (rand()&((opcode & 0x0F00) >> 8));//setting Vx to the result of a bitwise and operation on a random number and NN
		pc = pc + 2;//moving on to the next instruction
		break;
	}
	case 0xD000://decoding DXYN:Draws a sprite at coordinate (Vx, Vy) that has a width of 8 pixels and a height of N pixels.
	//Each row (N) of 8 pixels is read as bit coded starting from memory location I. 
	//The value of I doesn't change after the execution of the instruction. 
	{
		drawflag = true;//setting up the drawflag to allow the screen to change

		unsigned char xcoord = reg[(opcode & 0x0F00) >> 8];//getting Vx by taking the opcode, getting out the value of x needed, and shifting the total by 8 digits to get rid of the unneeded 0s
		unsigned char ycoord = reg[(opcode & 0x000F0) >> 4];//getting Vy by taking the opcode, getting out the value of y needed, and shifting the total by 4 digits to get rid of the unneeded 0s
		unsigned char rows = (opcode & 0x000F);//no need of any shifting of 0s for getting the rows
		unsigned char pixelrow;

		reg[15] = 0;

		for (int y = 0; y < rows; y++)//I'm trying to draw the sprite row by row, so starting the loop with y in order to increment the row I'm on after finishing with every pixel in each row
		{
			pixelrow = memory[I + y];//stores the current row inside pixelrow
			for (int x = 0; x < 8; x++)//checking every bit in each row(8 columns in each row)
			{
				if ((pixelrow&(0x80 >> x)) != 0)//taking the current row and then running through it to see
					//if the XOR operation would cause a set bit to change to 0
				{
					if (display[xcoord + x + ((ycoord + y) * 64)] == 1)//checking to see if the display pixels are 1
					//display is 64*32, so to get the sprite needed, multiply the total y by the number of pixels on
					//the screen to get your actual y coordinates(in reference to the ycoord)
					{
						reg[15] = 1;//setting up reg[15] to detect collision
					}
					display[xcoord + x + ((ycoord + y) * 64)] = display[xcoord + x + ((ycoord + y) * 64)] ^ 1;//doing the XOR operation	
				}
			}
		}
		pc = pc + 2;
		break;
	}
	case 0xE000://decoding the opcodes that start with an E
	{
		switch (opcode & 0x00FF)
		{
		case 0x009E: // decoding EX9E: Skips the next instruction if the key stored in V[x] is pressed
		{	if (key[reg[(opcode & 0x0F00) >> 8]] != 0)//if the button is pressed
		{
			pc += 4;//skip next instruction
		}
		else
		{
			pc += 2;
		}
		break;
		}
		case 0x00A1: // decoding EXA1: Skips the next instruction if the key stored in V[x] isn't pressed
		{
			if (key[reg[(opcode & 0x0F00) >> 8]] == 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		}
		}
		break;
		}
		//not mine, change it later
	case 0xF000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets Vx to the value of the delay timer
			reg[(opcode & 0x0F00) >> 8] = delay;
			pc += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in Vx
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					reg[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
		}
		break;

		case 0x0015: // FX15: Sets the delay timer to Vx
			delay = reg[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to Vx
			sound = reg[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: // FX1E: Adds Vx to I
			if (I + reg[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
				reg[0xF] = 1;
			else
				reg[0xF] = 0;
			I += reg[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in Vx. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			I = reg[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033: // FX33: Stores the Binary-coded decimal representation of Vx at the addresses I, I plus 1, and I plus 2
			memory[I] = reg[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (reg[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (reg[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // FX55: Stores V0 to Vx in memory starting at address I					
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = reg[i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065: // FX65: Fills V0 to Vx with values from memory starting at address I					
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				reg[i] = memory[I + i];

			// On the original interpreter, when the operation is done, I = I + x + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		}
		break;
	}

	default:

		cout << "Unknown opcode:" << "0x" << hex << opcode << "\n";

	}
	if (delay > 0)
	{
		delay--;
	}
	if (sound > 0)
	{
		sound--;
	}
	//used for debugging purposes
	cout << "Opcode:" << "0x" << hex << opcode << " at " << pc << " I: " << I << " SP: " << (int)sp<<" \n";
}

