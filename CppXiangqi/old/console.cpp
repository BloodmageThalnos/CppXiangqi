#include "console.h"

void setConsoleColor(int c, int b){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c+(b<<4));
}

int read(){
	register int res=0, c;
	while(c=getchar(), c<'0'||c>'9');
	do{
		res=res*10+(c^48);
	} while(c=getchar(), c>='0'&&c<='9');
	return res;
}