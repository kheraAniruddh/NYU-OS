#include<stdio.h>
#include<stdlib.h>

struct SymbolTable {
	char* symbol;
	int address;
	int moduleNo;
	int used;
	char const* error;
};


void printSymbolTable(SymbolTable s[100], int count) {
	printf("Symbol Table");
	for(int i=0;i<count;i++) {
		printf("\n%s=%d %s",s[i].symbol,s[i].address,s[i].error);
	}
	printf("\n");

}

int symbolLength(char* symbol) {
	if(strlen(symbol)>16) 
		return 1;
	else
		return 0;	
}
