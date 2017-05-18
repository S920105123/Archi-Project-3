#include <bits/stdc++.h>
using namespace std;

int btol(int);
void init();

map<string,int> R_code,I_code,J_code;

int main()
{
	int sp,word,i,buffer[12345];
	ofstream iimg,dimg;
	
	freopen("d_testcase.txt","r",stdin);
	dimg.open("dimage.bin",ios::binary);
	
	i=0;
	cin>>sp;
	while (cin>>word) {
		buffer[i++]=btol(word);
	}
	sp=btol(sp);
	word=btol(i);
	dimg.write((char*)&sp,4);
	dimg.write((char*)&word,4);
	dimg.write((char*)buffer,i*4);
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
