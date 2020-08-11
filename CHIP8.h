//this is how the chip8 was specified as far as I could tell

/*WHAT I LEARNED FROM THIS PROJECT:
1)Registers are an actual part of the processor. They are used mainly because of how fast they are compared to RAM, as well as other things
2)The program counter is a register that contains the address of the current instruction being executed 
3)The CPU contains instructions called opcode that allows the computer to know what operation needs to be completed 
*/

//this section last updated on 8/10/2020
/*HELPFUL LINKS
*https://en.wikipedia.org/wiki/CHIP-8#Virtual_machine_description
*http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
*http://www.cs.columbia.edu/~sedwards/classes/2016/4840-spring/designs/Chip8.pdf
*http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#dispcoords
http://emubook.emulation64.com/cpu.htm
https://www.cise.ufl.edu/~mssz/CompOrg/CDA-proc.html
http://www.emulator101.com/chip-8-emulator.html
https://erg.abdn.ac.uk/users/gorry/eg2068/course/dispatch.html
http://mattmik.com/files/chip8/mastering/chip8.html
-The starred links are some of the main websites I used to refer back to, especially in the beginning
-Some of the links are very repetitive, especially with the specs of the CHIP-8, but there are some extra tidbits of information that were important
from every link
-I also used Horowitz's "The Art of Electronics" as a really good way of referring back for information that I needed(not specifically
for the CHIP8 system, but just a bit of background reading before I try to go further with the project)
*/
#ifndef CHIP8_H
#define CHIP8_H

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

class CHIP8system
{
	unsigned char memory[4096] = {};//memory addresses need to be a byte in length
	unsigned char reg[16] = {};//V has to be a byte in length(reg[15] shouldn't be used, since it is used as a flag)
	unsigned short I;//the memory address register (which takes up 2 bytes) which is used for some opcodes with memory operations

	unsigned short pc;//program counter, contains the address of the current instruction

	unsigned short opcode;//the opcode has a length of 2 bytes

	unsigned char delay;//used for timing the events of games

	unsigned short stack[16] = {};//creating at most 16 stack levels
	unsigned char sp;//(stack pointer)keeps track of how much of the stack is taken up, takes up a byte of space


public:
	unsigned char display[SCREEN_HEIGHT * SCREEN_WIDTH] = {};//a part of the graphics, saves space as char instead of using int
	unsigned char sound;//used for sound effects(a nonzero value will give a beeping noise)
	unsigned char key[16] = {};//input also has to be a byte in length
	bool drawflag;//since sprites are only drawn every 16-20 cycles, this is going to tell us when the screen needs to be updated

	
//FUNCTIONS
	void init(); //used to initialize the CHIP8 system(clears all registers, memory, and the screen)
	bool loadgame(const char*);//loads the game into memory
	void emulatecycle();//emulates one cycle
};

#endif