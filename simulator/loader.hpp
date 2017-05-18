#ifndef LOADER_H
#define LOADER_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include "const.hpp"

struct Instruction {
	int opcode, funct;
	int rs,rt,rd;
	int immediate;
};

/* Memories */
extern int mem[1024];
extern Instruction inst[1024];

int btol(int target);  // Convertion between big/little endian.
short h_btol(short target);  // Convertion between big/little endian half word.
void parse(Instruction *inst, int word);    // Parse a word(big-endian) and store into *inst.
void print_inst(const Instruction *target); // Print an instruction. (for debugging)
void load_img(int &PC, int &num_inst, int &num_word, int &sp, int &pre_sp); // Call this function to load images.

#endif
