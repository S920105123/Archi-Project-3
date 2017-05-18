#include <string>
#include "const.hpp"

bool legal[64], legal_r[64];
std::string inst_str_r[64], inst_str[64];

void init_str_const()
{
	/* This function initialize inst_str with corresponding
	   instructions' name. These names are for debugging. */
	for (int i=0;i<64;i++) {
		inst_str_r[i] = inst_str[i] = "undefined";
	}
	
	/* R-format */
	inst_str_r[ADD]="add";
	inst_str_r[ADDU]="addu";
	inst_str_r[SUB]="sub";
	inst_str_r[AND]="and";
	inst_str_r[OR]="or";
	inst_str_r[XOR]="xor";
	inst_str_r[NOR]="nor";
	inst_str_r[NAND]="nand";
	inst_str_r[SLT]="slt";
	inst_str_r[SLL]="sll";
    inst_str_r[SRL]="srl";
    inst_str_r[SRA]="sra";
    inst_str_r[JR]="jr";
    inst_str_r[MULT]="mult";
    inst_str_r[MULTU]="multu";
    inst_str_r[MFHI]="mfhi";
    inst_str_r[MFLO]="mflo";
    
    /* Others */
    inst_str[ADDI]="addi";
    inst_str[ADDIU]="addiu";
    inst_str[LW]="lw";
    inst_str[LH]="lh";
    inst_str[LHU]="lhu";
    inst_str[LB]="lb";
    inst_str[LBU]="lbu";
    inst_str[SW]="sw";
    inst_str[SH]="sh";
    inst_str[SB]="sb";
    inst_str[LUI]="lui";
    inst_str[ANDI]="andi";
    inst_str[ORI]="ori";
    inst_str[NORI]="nori";
    inst_str[SLTI]="slti";
    inst_str[BEQ]="beq";
    inst_str[BNE]="bne";
    inst_str[BGTZ]="bgtz";
    inst_str[J]="j";
    inst_str[JAL]="jal";
    inst_str[HALT]="halt";
}

void init_const()
{
	/* Init bool legal */
	/* R-format */
	legal_r[ADD]=true;
	legal_r[ADDU]=true;
	legal_r[SUB]=true;
	legal_r[AND]=true;
	legal_r[OR]=true;
	legal_r[XOR]=true;
	legal_r[NOR]=true;
	legal_r[NAND]=true;
	legal_r[SLT]=true;
	legal_r[SLL]=true;
    legal_r[SRL]=true;
    legal_r[SRA]=true;
    legal_r[JR]=true;
    legal_r[MULT]=true;
    legal_r[MULTU]=true;
    legal_r[MFHI]=true;
    legal_r[MFLO]=true;
    
    /* Others */
    legal[ADDI]=true;
    legal[ADDIU]=true;
    legal[LW]=true;
    legal[LH]=true;
    legal[LHU]=true;
    legal[LB]=true;
    legal[LBU]=true;
    legal[SW]=true;
    legal[SH]=true;
    legal[SB]=true;
    legal[LUI]=true;
    legal[ANDI]=true;
    legal[ORI]=true;
    legal[NORI]=true;
    legal[SLTI]=true;
    legal[BEQ]=true;
    legal[BNE]=true;
    legal[BGTZ]=true;
    legal[J]=true;
    legal[JAL]=true;
    legal[HALT]=true;
}
