#include <fstream>
#include <cstdio>
#include <string>

extern int cycle;
static char err_buf[64];
FILE *ferr;
std::string err_str[5] = {
	"Write $0 Error\n",
	"Number Overflow\n",
	"Overwrite HI-LO registers\n",
	"Address Overflow\n",
	"Misalignment Error\n"
};

void error(int type)
{
	/* This function print error message in 
	   error_dump.rpt depends on error code "type" */
	//ferr<<"In cycle "<<cycle<<": "<<err_str[type];
	int len=sprintf(err_buf,"In cycle %d: %s", cycle, err_str[type].c_str());
	fwrite(err_buf,1,len,ferr);
}

void init_error()
{
	/* Initialize */
	//ferr.open("error_dump.rpt",std::ios_base::out);
	ferr=fopen("error_dump.rpt","wb");
}
