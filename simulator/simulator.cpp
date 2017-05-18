#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include "const.hpp"
#include "loader.hpp"
#include "datapath.hpp"
#include "error.hpp"
//#define DEBUG

FILE *fout;
int num_inst, num_word, cycle;
char buf[256];
bool stop_simulate;

inline void print_reg(int idx, bool &first) {
	/* Print changed registers into snapshot.rpt. */
	int len;
	static std::string reg_str[3]={"$HI","$LO","PC"};
	if (first) {
		//fout<<std::dec<<"cycle "<<cycle<<std::endl;
		len=sprintf(buf,"cycle %d\n",cycle);
		fwrite(buf,1,len,fout);
		first=false;
	}
	if (idx>31) {
		if (idx>33) {
			fwrite(reg_str[idx-32].c_str(),1,2,fout);
		} else {
			fwrite(reg_str[idx-32].c_str(),1,3,fout);
		}
	} else {
		//fout<<"$"<<std::dec<<std::setw(2)<<std::setfill('0')<<idx;
		len=sprintf(buf,"$%02d",idx);
		fwrite(buf,1,len,fout);
	}
	len=sprintf(buf,": 0x%08X\n",reg[idx]);
	//fout<<": 0x"<<std::hex<<std::uppercase<<std::setw(8)<<std::setfill('0')<<reg[idx]<<std::endl;
	fwrite(buf,1,len,fout);
}

void output()
{
	/* This function check whether register value is changed and output it if yes. */
	bool first=true;
	while (!change.empty()) {
		int idx=change.front();
		change.pop();
		if (reg[idx]!=pre_reg[idx]) {
			print_reg(idx,first);
			pre_reg[idx]=reg[idx];
		}
	}
	/* Check PC */
	if (PC!=pre_PC) {
		print_reg(34,first);
		pre_PC=PC;
	}
	if (!first) {
		buf[0]=buf[1]='\n';
		fwrite(buf,1,2,fout);
	}
}

void simulate()
{	
	/* This function control the flow of simulate. */
	int idx=(PC>>2), opcode=inst[idx].opcode, funct=inst[idx].funct;
	
	while (opcode!=HALT && !stop_simulate) {
		if (PC>=1024 || PC<0) {
			return;
		}
		cycle++;
		//std::cerr<<"cycle "<<std::dec<<cycle<<": ";
		//print_inst(&inst[idx]);
		if (opcode==0) {
			if (!legal_r[funct]) {
				std::cerr<<"illegal instruction found at 0x"<<std::setw(8)<<std::setfill('0')<<std::hex<<std::uppercase<<PC<<std::endl;
				return;
			}
			//fout<<inst_str_r[funct]<<"\n";
			(*R_func[funct])();
		} else {
			if (!legal[opcode]) {
				std::cerr<<"illegal instruction found at 0x"<<std::setw(8)<<std::setfill('0')<<std::hex<<std::uppercase<<PC<<std::endl;
				return;
			}
			//fout<<inst_str[opcode]<<"\n";
			(*func[opcode])();
		}
		if (!stop_simulate) {
			output();
		}
		
		idx=PC>>2;
		opcode=inst[idx].opcode;
		funct=inst[idx].funct;
	}
}

int main()
{	
	bool first=true;

	/* Initialization */	
	init_error();
	init_const();
	init_datapath();
	load_img(PC,num_inst,num_word,sp,pre_sp);
	fout=fopen("snapshot.rpt","wb");
	
	#ifdef DEBUG
	for (int i=PC>>2;i<(PC>>2)+num_inst;i++) {
		print_inst(&inst[i]);
	}
//	for (int i=0;i<num_word;i++) {
//		std::cerr<<std::hex<<mem[i]<<std::endl;
//	}
	#endif
	for (int i=0;i<35;i++) {
		print_reg(i,first);
	}
	buf[0]=buf[1]='\n';
	fwrite(buf,1,2,fout);
	
	/* Start simulation */
	simulate();
	fclose(ferr);
	fclose(fout);
}
