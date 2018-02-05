#include<stdio.h>
#include "SymbolTable.h"

struct Module{
	int startAddress;
	int endAddress;
	SymbolTable symbols[16];
	int noOfSymbsUsed;
	int defFlag;
	int useFlag;
	int progFlag;
	int defcount;
	int usecount;
	int progcount;
};


