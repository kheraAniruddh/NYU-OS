#include<stdio.h>
#include<stdlib.h>

struct MemoryMap {
	char address[4];
	char const* warning;
	int moduleNo;
	char * moduleWarning[100];
	int warningsNo;
};

//Print memory map
void printMemoryMap(MemoryMap m[100], int count) {
	for(int i=0;i<count;i++){
		printf("%03d: %s %s\n",i,m[i].address,m[i].warning);
		if(m[i].moduleNo != m[i+1].moduleNo ||i+1==count){
			int j=0;
			while(m[i+1].moduleWarning[j]!=NULL){
				printf("%s",m[i+1].moduleWarning[j]); j++;}
		}
	}
	printf("\n");
}
