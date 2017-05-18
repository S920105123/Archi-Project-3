#include <bits/stdc++.h>
using namespace std;

int btol(int);
void init();

map<string,int> R_code,I_code,J_code;

int main()
{
	int PC,op,rs,rt,rd,imm,funct,word,buffer[1024],i;
	string inst;
	ofstream iimg,dimg;
	
	init();
	freopen("i_testcase.txt","r",stdin);
	iimg.open("iimage.bin",ios::binary);
	
	i=0;
	cin>>PC;
	while (cin>>inst) {
		cout<<"Eat "<<inst<<endl;
		if (R_code.find(inst)!=R_code.end()) {
			cin>>dec>>rs>>rt>>rd>>hex>>imm;
		cout<<dec<<rs<<" "<<rt<<" "<<rd<<" "<<hex<<imm<<endl;
			word=(rs<<21)|(rt<<16)|(rd<<11)|(imm<<6)|R_code[inst];
		} else if (I_code.find(inst)!=I_code.end()) {
			cin>>dec>>rs>>rt>>hex>>imm;
			cout<<dec<<rs<<" "<<rt<<" "<<hex<<imm<<endl;
			word=(I_code[inst]<<26)|(rs<<21)|(rt<<16)|(imm&0xFFFF);
		} else if (J_code.find(inst)!=J_code.end()) {
			cin>>hex>>imm;
			word=(J_code[inst]<<26)|(imm&0x3FFFFFF);
		} else if (inst=="halt") {
			word=0x3F<<26;
		} else {
			cerr<<"Write an illegal inst!! "<<inst<<endl;
			word=0x12<<26;
		}
		word=btol(word);
		buffer[i]=word;
		i++;
	}
	PC=btol(PC);
	word=btol(i);
	iimg.write((char*)&PC,4);
	iimg.write((char*)&word,4);
	iimg.write((char*)buffer,i*4);
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


void init()
{
	R_code["add"]=0x20;
	R_code["addu"]=0x21;
	R_code["sub"]=0x22;
	R_code["and"]=0x24;
	R_code["or"]=0x25;
	R_code["xor"]=0x26;
	R_code["nor"]=0x27;
	R_code["nand"]=0x28;
	R_code["slt"]=0x2A;
	R_code["sll"]=0x00;
	R_code["srl"]=0x02;
	R_code["sra"]=0x03;
	R_code["jr"]=0x08;
	R_code["mult"]=0x18;
	R_code["multu"]=0x19;
	R_code["mfhi"]=0x10;
	R_code["mflo"]=0x12;
	
	I_code["addi"]=0x08;
	I_code["addiu"]=0x09;
	I_code["lw"]=0x23;
	I_code["lh"]=0x21;
	I_code["lhu"]=0x25;
	I_code["lb"]=0x20;
	I_code["lbu"]=0x24;
	I_code["sw"]=0x2B;
	I_code["sh"]=0x29;
	I_code["sb"]=0x28;
	I_code["lui"]=0x0F;
	I_code["andi"]=0x0C;
	I_code["ori"]=0x0D;
	I_code["nori"]=0x0E;
	I_code["slti"]=0x0A;
	I_code["beq"]=0x04;
	I_code["bne"]=0x05;
	I_code["bgtz"]=0x07;
	
	J_code["j"]=0x02;
	J_code["jal"]=0x03;
}
