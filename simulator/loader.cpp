#include <iostream>
#include <iomanip>
#include <cstdio>
#include "const.hpp"
#include "loader.hpp"

/* Memories */
int mem[1024];
Instruction inst[1024];

inline bool un_signed(int opcode)
{
	/* Whether I-format inst immediate is unsigned or not. */
	return opcode==ANDI || opcode==ORI || opcode==NORI;
}
int btol(int target){
	/* This function conducts the convertion between
	   big-endian numbers and little-endian numbers. */
	char *bytes=(char*)&target, temp;
	temp=bytes[0];
	bytes[0]=bytes[3];
	bytes[3]=temp;
	temp=bytes[1];
	bytes[1]=bytes[2];
	bytes[2]=temp;
	return *(int*)bytes;
}

short h_btol(short target)
{
	/* This function conducts the convertion between
	   big-endian numbers and little-endian numbers. */
	char *bytes=(char*)&target, temp;
	temp=bytes[0];
	bytes[0]=bytes[1];
	bytes[1]=temp;
	return *(short*)bytes;
}

void parse(Instruction *inst, int word)
{
	/* This function parse a binary-formatted big-endian 
	   instruction and store all the information into *inst. */
	word=btol(word);
	int mask5=0x1F, mask6=0x3F, mask16=0xFFFF, mask26=0x3FFFFFF;
	inst->opcode=(word>>26)&mask6;
	if (inst->opcode==0) {
		inst->rs=(word>>21)&mask5;
		inst->rt=(word>>16)&mask5;
		inst->rd=(word>>11)&mask5;
		inst->immediate=(word>>6)&mask5;
		inst->funct=word&mask6;
	} else {
		if (inst->opcode==J || inst->opcode==JAL) {
			inst->immediate=word&mask26;
		} else if (inst->opcode!=HALT) {
			inst->rs=(word>>21)&mask5;
			inst->rt=(word>>16)&mask5;
			inst->immediate=word&mask16;
			if (!un_signed(inst->opcode) && ((inst->immediate>>15)&1)) {
				inst->immediate|=0xFFFF0000;
			} 
		}
	}
}

void print_inst(const Instruction *target)
{
	/* This function print an instruction. (for debugging) */
	static bool first=true;
	if (first) {
		first=false;
		init_str_const();
	}
	if (target->opcode==0) {
		std::cerr<<inst_str_r[target->funct];
		std::cerr<<std::dec<<" "<<target->rd<<" "<<target->rs<<" "<<target->rt<<" "<<target->immediate;
	} else {
		std::cerr<<inst_str[target->opcode];
		if (target->opcode==J || target->opcode==JAL) {
			std::cerr<<std::hex<<" "<<target->immediate;
		} else if (target->opcode!=HALT) {
			std::cerr<<std::dec<<" "<<target->rt<<" "<<target->rs<<" "<<target->immediate;
		}
	}
	std::cout<<std::endl;
}

void load_img(int &PC, int &num_inst, int &num_word, int &sp, int &pre_sp)
{
	/* This function load iimage & dimage then reset instruction set(inst),
	   stack(stk), initial PC, number of instructions and stack pointer(sp).  */
	int word,i;
	FILE *i_img, *d_img;
	
	d_img=fopen("dimage.bin","rb");
	i_img=fopen("iimage.bin","rb");
	
	/* Load instruction image. */
	fread((char*)&PC,4,1,i_img);
	fread((char*)&num_inst,4,1,i_img);
	PC=btol(PC);
	num_inst=btol(num_inst);
	for (i=PC/4;i<PC/4+num_inst;i++) {
		fread((char*)&word,4,1,i_img);
		parse(&inst[i],word);
	}
	
	/* Load memory data image. */
	fread((char*)&sp,4,1,d_img);
	fread((char*)&num_word,4,1,d_img);
	sp=btol(sp);
	pre_sp=sp;
	num_word=btol(num_word);
	for (i=0;i<num_word;i++) {
		fread((char*)&mem[i],4,1,d_img);
	}
	
	fclose(i_img);
	fclose(d_img);
}
