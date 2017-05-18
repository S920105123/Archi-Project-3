#include "error.hpp"
#include "datapath.hpp"
#include "loader.hpp" 
#include <climits>
#include <queue>

extern bool stop_simulate;

/* Convesion between big/little endian */
extern int btol(int target);
extern short h_btol(short target);

/* Registers */
std::queue<int> change;
int reg[35], HI=32, LO=33, &PC=reg[34], &sp=reg[29];
int pre_reg[35], &pre_PC=pre_reg[34], &pre_sp=pre_reg[29];
bool hilo_used=false;

/* Function pointers */
void (*R_func[64])();
void (*func[64])();

/* R-format instructions */
void inst_add() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]+pre_reg[rt];
		change.push(rd);
	}
	if ((pre_reg[rs]>0&&pre_reg[rt]>0&&pre_reg[rs]>INT_MAX-pre_reg[rt]) ||
	    (pre_reg[rs]<0&&pre_reg[rt]<0&&pre_reg[rs]<INT_MIN-pre_reg[rt])) {
			//std::cerr<<"ovf\n";
			error(NUM_OVF);
	}
	
	PC=PC+4;
}
void inst_addu() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]+pre_reg[rt];
		change.push(rd);
	}
	PC=PC+4;
}
void inst_sub() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]-pre_reg[rt];
		change.push(rd);
	}
	if ((pre_reg[rs]>0&&-pre_reg[rt]>0&&pre_reg[rs]>INT_MAX+pre_reg[rt]) ||
	    (pre_reg[rs]<0&&-pre_reg[rt]<0&&pre_reg[rs]<INT_MIN+pre_reg[rt])) {
			error(NUM_OVF);
	}
	PC=PC+4;
}
void inst_and() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]&pre_reg[rt];
		change.push(rd);
	}
	PC=PC+4;
}
void inst_or() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]|pre_reg[rt];
		change.push(rd);
	}
	PC=PC+4;
}
void inst_xor() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]^pre_reg[rt];
		change.push(rd);
	}
	PC=PC+4;
}
void inst_nor() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=~(pre_reg[rs]|pre_reg[rt]);
		change.push(rd);
	}
	PC=PC+4;
}
void inst_nand() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=~(pre_reg[rs]&pre_reg[rt]);
		change.push(rd);
	}
	PC=PC+4;
}
void inst_slt() {
	int rd=inst[PC>>2].rd, rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rs]<pre_reg[rt];
		change.push(rd);
	}
	PC=PC+4;
}
void inst_sll() {
	int rd=inst[PC>>2].rd, rt=inst[PC>>2].rt, immediate=inst[PC>>2].immediate;
	if (rd==0 && (rt!=0||immediate!=0)) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rt]<<immediate;
		change.push(rd);
	}
	PC=PC+4;
}
void inst_srl() {
	int rd=inst[PC>>2].rd, rt=inst[PC>>2].rt, immediate=inst[PC>>2].immediate;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		if (immediate!=0) {	
			change.push(rd);
			reg[rd]=pre_reg[rt]>>1;
			reg[rd]=reg[rd]&0x7FFFFFFF;
			reg[rd]=reg[rd]>>(immediate-1);
		} else {
			change.push(rd);
			reg[rd]=pre_reg[rt];
		}
	}
	PC=PC+4;
}
void inst_sra() {
	int rd=inst[PC>>2].rd, rt=inst[PC>>2].rt, immediate=inst[PC>>2].immediate;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[rt]>>immediate;
		change.push(rd);
	}
	PC=PC+4;
}
void inst_jr() {
	int rs=inst[PC>>2].rs;
	PC=pre_reg[rs];
}
void inst_mult() {
	int rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	long long res=pre_reg[rs];
	if (hilo_used) {
		error(OVERWRITE_HILO);
	} else {
		hilo_used=true;
	}
	res=res*pre_reg[rt];
	reg[HI]=res>>32;
	reg[LO]=res&0x00000000FFFFFFFF;
	change.push(HI);
	change.push(LO);
	PC=PC+4;
}
void inst_multu() {
	int rs=inst[PC>>2].rs, rt=inst[PC>>2].rt;
	unsigned long long res=(unsigned int)pre_reg[rs];
	res=res*(unsigned int)pre_reg[rt];
	if (hilo_used) {
		error(OVERWRITE_HILO);
	} else {
		hilo_used=true;
	}
	reg[HI]=res>>32;
	reg[LO]=res&(long long)0x00000000FFFFFFFF;
	change.push(HI);
	change.push(LO);
	PC=PC+4;
}
void inst_mfhi() {
	int rd=inst[PC>>2].rd;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[HI];
		change.push(rd);
	}
	hilo_used=false;
	PC=PC+4;
}
void inst_mflo() {
	int rd=inst[PC>>2].rd;
	hilo_used=false;
	if (rd==0) {
		error(WRITE_ZERO);
	} else {
		reg[rd]=pre_reg[LO];
		change.push(rd);
	}	
	PC=PC+4;
}

/* I-format instructions */
void inst_addi() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=pre_reg[rs]+immediate;
		change.push(rt);
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
			error(NUM_OVF);
	}
	PC=PC+4;
}
void inst_addiu() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=pre_reg[rs]+immediate;
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_lw() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	int offset=immediate+pre_reg[rs];
	if (rt==0) {
		error(WRITE_ZERO);
		block=true;
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate+3>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if ((pre_reg[rs]+immediate)%4!=0) {
		error(DATA_MISALIGNED);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		reg[rt]=mem[offset>>2];
		reg[rt]=btol(reg[rt]);
		change.push(rt);
	}
	PC=PC+4;
}
void inst_lh() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	int offset=immediate+pre_reg[rs];
	if (rt==0) {
		error(WRITE_ZERO);
		block=true;
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate+1>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if ((pre_reg[rs]+immediate)%2!=0) {
		error(DATA_MISALIGNED);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		reg[rt]=*((short*)mem+(offset>>1));
		reg[rt]=h_btol((short)reg[rt]);
		change.push(rt);
	}
	PC=PC+4;
}
void inst_lhu() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	int offset=immediate+pre_reg[rs];
	if (rt==0) {
		error(WRITE_ZERO);
		block=true;
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	} 
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate+1>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if ((pre_reg[rs]+immediate)%2!=0) {
		error(DATA_MISALIGNED);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		reg[rt]=*((unsigned short*)mem+(offset>>1));
		reg[rt]=(unsigned short)h_btol(reg[rt]);
		reg[rt]=reg[rt]&0x0000FFFF;  // Just in case.
		change.push(rt);
	}
	PC=PC+4;
}
void inst_lb() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
		block=true;
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if (!block){
		reg[rt]=*((char*)mem+pre_reg[rs]+immediate);
		change.push(rt);
	}
	PC=PC+4;
}
void inst_lbu() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
		block=true;
	}
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if (!block){
		reg[rt]=*((unsigned char*)mem+pre_reg[rs]+immediate);
		reg[rt]=reg[rt]&0x000000FF;  // Just in case.
		change.push(rt);
	}
	PC=PC+4;
}
void inst_sw() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	int offset=immediate+pre_reg[rs];
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate+3>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if ((pre_reg[rs]+immediate)%4!=0) {
		error(DATA_MISALIGNED);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		mem[offset>>2]=btol(reg[rt]);
	}
	PC=PC+4;
}
void inst_sh() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	int offset=immediate+pre_reg[rs];
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate+1>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if ((pre_reg[rs]+immediate)%2!=0) {
		error(DATA_MISALIGNED);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		*((short*)mem+(offset>>1))=h_btol(reg[rt]&0x0000FFFF);
	}
	PC=PC+4;
}
void inst_sb() {
	bool block=false;
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if ((pre_reg[rs]>0&&immediate>0&&pre_reg[rs]>INT_MAX-immediate) ||
	    (pre_reg[rs]<0&&immediate<0&&pre_reg[rs]<INT_MIN-immediate)) {
		error(NUM_OVF);
	}
	if (pre_reg[rs]+immediate<0 || pre_reg[rs]+immediate>=1024)  {
		error(MEM_ADDR_OVF);
		stop_simulate=true;
		block=true;
	}
	if (!block) {
		*((char*)mem+pre_reg[rs]+immediate)=reg[rt]&0x000000FF;
	}
	PC=PC+4;
}
void inst_lui() {
	int rt=inst[PC>>2].rt, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=immediate<<16;
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_andi() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=pre_reg[rs]&immediate;
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_ori() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=pre_reg[rs]|immediate;
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_nori() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=~(pre_reg[rs]|immediate);
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_slti() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (rt==0) {
		error(WRITE_ZERO);
	} else {
		reg[rt]=pre_reg[rs]<immediate;
		change.push(rt);
	} 
	PC=PC+4;
}
void inst_beq() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (reg[rs]==reg[rt]) {
		PC=PC+4*immediate;
	}
	PC=PC+4;
}
void inst_bne() {
	int rt=inst[PC>>2].rt, rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (reg[rs]!=reg[rt]) {
		PC=PC+4*immediate;
	}
	PC=PC+4;
}
void inst_bgtz() {
	int rs=inst[PC>>2].rs, immediate=inst[PC>>2].immediate;
	if (reg[rs]>0) {
		PC=PC+4*immediate;
	}
	PC=PC+4;
}
void inst_j() {
	int immediate=inst[PC>>2].immediate;
	PC=((PC+4)&0xF0000000)|(4*immediate);
}
void inst_jal() {
	int immediate=inst[PC>>2].immediate;
	reg[31]=PC+4;
	change.push(31);
	PC=((PC+4)&0xF0000000)|(4*immediate);
}

void init_datapath() {
	/* Init function pointers. */
	
	/* R-format */
	R_func[ADD]=&inst_add;
	R_func[ADDU]=&inst_addu;
	R_func[SUB]=&inst_sub;
	R_func[AND]=&inst_and;
	R_func[OR]=&inst_or;
	R_func[XOR]=&inst_xor;
	R_func[NOR]=&inst_nor;
	R_func[NAND]=&inst_nand;
	R_func[SLT]=&inst_slt;
	R_func[SLL]=&inst_sll;
	R_func[SRL]=&inst_srl;
	R_func[SRA]=&inst_sra;
	R_func[JR]=&inst_jr;
	R_func[MULT]=&inst_mult;
	R_func[MULTU]=&inst_multu;
	R_func[MFHI]=&inst_mfhi;
	R_func[MFLO]=&inst_mflo;
	
	/* I-format and J-format */
	func[ADDI]=&inst_addi;
	func[ADDIU]=&inst_addiu;
	func[LW]=&inst_lw;
	func[LH]=&inst_lh;
	func[LHU]=&inst_lhu;
	func[LB]=&inst_lb;
	func[LBU]=&inst_lbu;
	func[SW]=&inst_sw;
	func[SH]=&inst_sh;
	func[SB]=&inst_sb;
	func[LUI]=&inst_lui;
	func[ANDI]=&inst_andi;
	func[ORI]=&inst_ori;
	func[NORI]=&inst_nori;
	func[SLTI]=&inst_slti;
	func[BEQ]=&inst_beq;
	func[BNE]=&inst_bne;
	func[BGTZ]=&inst_bgtz;
	func[J]=&inst_j;
	func[JAL]=&inst_jal;
}
