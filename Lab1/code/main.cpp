#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "Module.h"
#include "MemoryMap.h"
#include<iostream>

#define MAX_INSTR 512
#define DEF_OR_USE_LIST 16

int moduleCount = 0;
int symbolCount = 0;
int memoryMapCount = 0;
int line =1;
int totalLines=1;
int offset=1;
int startAdd = 0;
Module modules[100];
SymbolTable symbols[256];
MemoryMap memoryMapItems[256];


void parseError(int errcode, int line,int offset) {
  static char const* errstr[] = {
      "NUM_EXPECTED",
      "SYM_EXPECTED",
      "ADDR_EXPECTED",
      "SYM_TOO_LONG",
      "TOO_MANY_DEF_IN_MODULE",
      "TOO_MANY_USE_IN_MODULE",
      "TOO_MANY_INSTR",
  };
  printf("Parse Error line %d offset %i: %s\n",line,offset, errstr[errcode]);
  exit(1);
}

size_t stringfinder(std::string str, std::string sub);



int illegalOpcode(int num) {
	int digitcount = 0;	
	while(num!=0) {
		digitcount++;
		num /=10;
	}
	if(digitcount > 4)
		return 1;
	return 0;
	
}



void checkSymbolUsage(char* token) {
	int i = 0;
	while(i<symbolCount) {
		if(strcmp(symbols[i].symbol,token)==0) 
			symbols[i].used=1;
		i++;
	}	
}



void printUnusedSymbols() {
	int i = 0;
	while(i<symbolCount) {
		if(symbols[i].used==0)
			printf("Warning: Module %d: %s was defined but never used\n",symbols[i].moduleNo+1,symbols[i].symbol);
		i++;
	}
}



int checkSymbolDef(int operand) {
	int flag=0;	
	for(int j=0;j<symbolCount;j++){	
			if(strcmp(modules[moduleCount].symbols[operand].symbol,symbols[j].symbol)==0 && operand<modules[moduleCount].noOfSymbsUsed){
				flag=1;}
			
	}
	return flag;
}

void checkSymbolApp() {	
	int k=0;
	for(int i=0;i<modules[moduleCount].noOfSymbsUsed;i++){
		if(modules[moduleCount].symbols[i].used!=1) {			
			char tem[1000];
			strcpy(tem,"\0");
			strcat(tem,"Warning: Module ");
			char c[10];
			sprintf(c,"%d",moduleCount+1);
			strcat(tem,c);
			strcat(tem,": ");
			strcat(tem,modules[moduleCount].symbols[i].symbol);
			strcat(tem, " appeared in the uselist but was not actually used\n");			
			memoryMapItems[memoryMapCount].moduleWarning[k] = tem;			
			k++;
		}
	}
		
}		

int findAddress(char * symbol) {
	int address=0;
	for(int i = 0;i<symbolCount;i++) {
		if(strcmp(symbols[i].symbol,symbol)==0){
			address=symbols[i].address;
		}
	}
	return address;
}


void checkAddSize(int progCount) {
  int i = 0;
  while(i<symbolCount) {
      if(symbols[i].moduleNo == moduleCount) {
	  if(symbols[i].address-modules[moduleCount].startAddress > progCount) {
	      printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n",moduleCount+1,symbols[i].symbol,symbols[i].address-modules[moduleCount].startAddress ,progCount-1);
	      symbols[i].address=0;
	  }
      }
      i++;
  }
}

int ifDup(char * token) {
  int i =0;
  int flag = 0 ;
  while(i<symbolCount) {
      if(strcmp(token, symbols[i].symbol) == 0) {
	  flag=1;
	  symbols[i].error="Error: This variable is multiple times defined; first value used";
      }
      i++;
  }
  if(flag==1)
    return 1;
  else
    return 0;
}


void firstPass(FILE* fp) {
  // Determine file size
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  char * buff =new char[size];
  rewind(fp);
  fread(buff, sizeof(char),size, fp);
  char *endLine;
  char *ltoken = strtok_r(buff,"\n",&endLine);
  int start=1;
  modules[moduleCount].defFlag=modules[moduleCount].useFlag=modules[moduleCount].progFlag=0;
  modules[moduleCount].defcount=modules[moduleCount].usecount=modules[moduleCount].progcount=-1;
  while(ltoken != NULL) {
      char *endToken;
      std::string str(ltoken);
      char * ctoken = strtok_r(ltoken,"\t ",&endToken);
      while(ctoken != NULL) { 
	  //Definition list check
	  if((modules[moduleCount].defFlag==0 && ctoken!=NULL) || ( modules[moduleCount].defFlag==0 && line==totalLines)){
	      if(modules[moduleCount].defcount==-1){
		  if(atoi(ctoken) || isdigit(ctoken[0])) {
		      	modules[moduleCount].defcount = atoi(ctoken);
		   }
		      else 
			  parseError(0,line,1);
		      if(modules[moduleCount].defcount >DEF_OR_USE_LIST) {
			  std::string sub(ctoken);
			  parseError(4,line,stringfinder(str,sub));
			}
			ctoken= strtok_r(NULL,"\t ",&endToken);	
			modules[moduleCount].defcount *=2;
		}

		  while((modules[moduleCount].defcount>0 && ctoken!=NULL)|| (modules[moduleCount].defcount>0 && line==totalLines)) {
		      if(modules[moduleCount].defcount%2 == 0) {
			  if(ctoken ==NULL) 
				parseError(1,line,str.length()+1); 
			else 
			  if(!atoi(ctoken)){
			      if(ifDup(ctoken)==0){
				  symbols[symbolCount].symbol = ctoken;
				  if(symbolLength(ctoken)) {
					 std::string sub(ctoken);parseError(3,line,stringfinder(str,sub));
					}
				  symbols[symbolCount].moduleNo = moduleCount;
				  symbols[symbolCount].used=0;
				  symbols[symbolCount].error = "";
				  ctoken= strtok_r(NULL,"\t ",&endToken);
			      }
			      else{ ctoken= strtok_r(NULL,"\t ",&endToken); ctoken= strtok_r(NULL,"\t ",&endToken);modules[moduleCount].defcount--;
			      }
			  }
			}
		      else {
			      if(ctoken==NULL)  parseError(0,line,str.length()+1);
			      if(atoi(ctoken) || isdigit(ctoken[0])) {
				  symbols[symbolCount].address = atoi(ctoken) + startAdd;
				  symbolCount++;
				  ctoken= strtok_r(NULL,"\t ",&endToken);
			      }
				else{ std::string sub(ctoken);
				 parseError(0,line,stringfinder(str,sub));}
			  }
		      modules[moduleCount].defcount--;
		}	
		  if(modules[moduleCount].defcount == 0){
		      modules[moduleCount].defFlag=1;
		  }
	      }
	      // Check Use List
	      if((modules[moduleCount].useFlag==0 && ctoken!=NULL)|| (modules[moduleCount].useFlag==0 &&line==totalLines)){
		  if(modules[moduleCount].usecount==-1 ){
		      if(ctoken==NULL)
			parseError(0,line,1);
		      if(atoi(ctoken) || isdigit(ctoken[0])) {
			  modules[moduleCount].usecount = atoi(ctoken);
			  if(modules[moduleCount].usecount >DEF_OR_USE_LIST)
			    {  std::string sub(ctoken);parseError(5,line,stringfinder(str,sub));}
			  ctoken= strtok_r(NULL,"\t ",&endToken);
		      }
		  }
		  while((modules[moduleCount].usecount > 0 && ctoken!=NULL)|| (modules[moduleCount].usecount > 0 && line==totalLines)) { 
		      if(!atoi(ctoken)) {
			  ctoken= strtok_r(NULL,"\t ",&endToken);
			  modules[moduleCount].usecount--;
		      }
		      else{
			  std::string sub(ctoken);
			  parseError(1,line,stringfinder(str,sub));
		      }
		  }
		  if(modules[moduleCount].usecount ==0) {
		      modules[moduleCount].useFlag=1;
		  }
	      }
	      // Check prog List
	      if((modules[moduleCount].progFlag==0 && ctoken!=NULL)|| (modules[moduleCount].progFlag==0 &&line==totalLines)){

		  if(modules[moduleCount].progcount==-1 ){
		      if(atoi(ctoken) || isdigit(ctoken[0])) {
			  modules[moduleCount].progcount = atoi(ctoken);
			  modules[moduleCount].startAddress = startAdd;
			  checkAddSize(modules[moduleCount].progcount);
			  modules[moduleCount].endAddress = modules[moduleCount].progcount;
			  startAdd += modules[moduleCount].endAddress;
				
			  if(startAdd>MAX_INSTR){
			      std::string sub(ctoken);
			      parseError(6,line+1,stringfinder(str,sub));
			  }
			  modules[moduleCount].progcount *= 2;
			  ctoken= strtok_r(NULL,"\t ",&endToken);
		      }
		      else{
			  parseError(0,line,1);
		      }
		  }
		  while((modules[moduleCount].progcount > 0  && ctoken!=NULL)|| (modules[moduleCount].progcount > 0  && line==totalLines)) {
		      if(modules[moduleCount].progcount%2 == 0) {
			  if(ctoken==NULL){
			      parseError(2,line,str.length()+1);
			  }
			  if(strcmp(ctoken,"A")==0 || strcmp(ctoken,"E")==0 || strcmp(ctoken,"I")==0 || strcmp(ctoken,"R")==0) {
			  }
		      }
		      ctoken = strtok_r(NULL,"\t ",&endToken);
		      modules[moduleCount].progcount--;
		  }
		  if(modules[moduleCount].progcount==0){
		      modules[moduleCount].progFlag=1;
		  }
	      }
	      if(modules[moduleCount].progFlag==1 && modules[moduleCount].useFlag==1 && modules[moduleCount].defFlag==1) {
		  moduleCount++;
		  modules[moduleCount].defFlag=modules[moduleCount].useFlag=modules[moduleCount].progFlag=0;
		  modules[moduleCount].defcount=modules[moduleCount].usecount=modules[moduleCount].progcount=-1;
	      }
	  }
	  line++; 
	  ltoken = strtok_r(NULL,"\n",&endLine);
      }
      printSymbolTable(symbols,symbolCount);
      moduleCount=0;
  }

  size_t stringfinder(std::string str, std::string sub) {
    size_t pos=str.find(sub);
    return pos+1;
  }

void secondPass(char * buff) {
	printf("\nMemory Map\n");
	char * token = strtok(buff,",");	
	while(token != NULL) {		
		
		int defcount = atoi(token);
		defcount *=2;
		while(defcount > 0) {
			token = strtok(NULL,",");
			defcount--;				
		}
		
		token = strtok(NULL,",");
		int usecount = atoi(token);
		int symbolIndex=0;
		while(usecount > 0) {
			token = strtok(NULL,",");
			checkSymbolUsage(token);
			modules[moduleCount].symbols[symbolIndex].symbol=token;
			symbolIndex++;	
			usecount--;
		}
		modules[moduleCount].noOfSymbsUsed=symbolIndex;
		
		token = strtok(NULL,",");	
		int progcount = atoi(token);
		progcount *=2;
		char const*addressSymbol;
		while(progcount>0) {
			memoryMapItems[memoryMapCount].moduleNo = moduleCount;
			token = strtok(NULL,",");
			if(progcount%2==0) {
				addressSymbol=token;				
			}
			else {
				if(strcmp(addressSymbol,"A")==0) {
					int num = atoi(token);
					int operand = num%1000;
					int opcode = num/1000;
					if(illegalOpcode(num)) {
					 	num= 9999;
						memoryMapItems[memoryMapCount].warning = "Error: Illegal immediate value; treated as 9999";	
					}
					else {
					if(operand>MAX_INSTR){
						num= opcode*1000;
						memoryMapItems[memoryMapCount].warning = "Error: Absolute address exceeds machine size; zero used";
					}
					else
						memoryMapItems[memoryMapCount].warning = "";						
					}					
					sprintf(memoryMapItems[memoryMapCount].address,"%04d",num);									
				}
				else
				if(strcmp(addressSymbol,"E")==0) {
					int num = atoi(token);
					int operand = num%1000;
					int opcode = num/1000;
					if(illegalOpcode(num)) {
					 	num= 9999;
						memoryMapItems[memoryMapCount].warning = "Error: Illegal opcode; treated as 9999";	
					}
					else {
					modules[moduleCount].symbols[operand].used=1;
					if(operand>modules[moduleCount].noOfSymbsUsed-1) {
						memoryMapItems[memoryMapCount].warning ="Error: External address exceeds length of uselist; treated as immediate";
						num = atoi(token);
					}else{
					int defret=checkSymbolDef(operand);
					if(defret==0)	{
						char temp[2000];
						strcpy(temp,"\0");
						strcat(temp,"Error: ");
						strcat(temp,modules[moduleCount].symbols[operand].symbol);
						strcat(temp," is not defined; zero used");
						memoryMapItems[memoryMapCount].warning = temp;				
					}
					else {
						 num = opcode*1000+findAddress(modules[moduleCount].symbols[operand].symbol);
						 memoryMapItems[memoryMapCount].warning = "";
					}	
					}
					}
					sprintf(memoryMapItems[memoryMapCount].address,"%04d",num);		
				}
				else
				if(strcmp(addressSymbol,"I")==0) {
					int num = atoi(token);
					if(illegalOpcode(num)) {
					 	num= 9999;
						memoryMapItems[memoryMapCount].warning = "Error: Illegal immediate value; treated as 9999";	
					}
					else {
						memoryMapItems[memoryMapCount].warning ="";
					}
					sprintf(memoryMapItems[memoryMapCount].address,"%04d",num);					
				}
				else
				if(strcmp(addressSymbol,"R")==0) {
					int num = atoi(token);
					int operand = num%1000;
					int opcode = num/1000;
					if(illegalOpcode(num)) {
					 	num= 9999;
						memoryMapItems[memoryMapCount].warning = "Error: Illegal opcode; treated as 9999";	
					}
					else {
					if(operand>modules[moduleCount].endAddress){
						memoryMapItems[memoryMapCount].warning = "Error: Relative address exceeds module size; zero used";
						num = opcode*1000+modules[moduleCount].startAddress;
					}
					else{
						num+=modules[moduleCount].startAddress;
						memoryMapItems[memoryMapCount].warning = "";	
					}
					}					
					sprintf(memoryMapItems[memoryMapCount].address,"%04d",num);	
										
				}
				else
				 printf("\nSome prob in A E I R: pass2\n");
				memoryMapCount++;			
			}
			progcount--;
		}
		checkSymbolApp();
		moduleCount++;
		token =strtok(NULL,",");
	}
	printMemoryMap(memoryMapItems,memoryMapCount);			    
	printUnusedSymbols();
}



void customParser(FILE* fp) {
	char buff[255];
	fgets(buff, 255, (FILE*)fp);
	char buffer[2000];
	strcpy(buffer,"\0");
	while(!feof(fp)) {
		char *token = strtok(buff," \n\t");
		while(token != NULL) {
			strcat(buffer,token);
			strcat(buffer,",");
			token = strtok(NULL, " \n\t");
		}
		fgets(buff, 255, (FILE*)fp);
	}
	secondPass(buffer);	
}
void calTL(FILE* fp) {
char buff[255];
	fgets(buff, 255, (FILE*)fp);
	int i=0;
	while(!feof(fp)) {
		i++;
		fgets(buff, 255, (FILE*)fp);
	}
	totalLines =i;
}

  int main(int argc, char *argv[]) {
    char *buf;
    if(argc == 2) {
	buf =argv[1];
	char buff[255] ="";
	FILE *fp3 = NULL;
	if(!(fp3=fopen(buf,"r"))) {
	    printf("Unable to open file, please check the path specified!\n");
	}
	else {
	    calTL(fp3);
	    fclose(fp3);
	}
	FILE *fp = NULL;
	if(!(fp=fopen(buf,"r"))) {
	    printf("Unable to open file, please check the path specified!\n");
	}
	else {
	    firstPass(fp);
	    fclose(fp);
	}
	FILE *fp2 = NULL;
	if(!(fp2=fopen(buf,"r"))) {
	    printf("Unable to open file, please check the path specified!\n");
	}
	else {
	    customParser(fp2);
	    fclose(fp2);
	}
    }
    else {
	printf("Invalid arguments given. Please enter file path as argument.\n");
    }
    return 0;
  }
