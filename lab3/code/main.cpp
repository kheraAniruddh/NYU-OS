#include<stdio.h>
#include<getopt.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<vector>
#include<list>
#include<iostream>

using namespace std;
int getRndmNo(int no);

class Instruction {
public:
	Instruction() {}
	int readWrite;
	int pageNo;
	long counter;
};

list<Instruction*> instructs ;

class Frame {
public:
	int pageNo;
	int frameIndex;
	Frame() { pageNo=-1; frameIndex=-1;}
};

vector<Frame*> frameTable;

struct Pte {
	unsigned present:1;
	unsigned modified:1;
	unsigned refereneced:1;
	unsigned pagedOut:1;
	Frame* frame;
}pt[64];

class Paging {

public:
	Paging() {}
	virtual Frame* getVictimFrame(){}
	virtual void incPt(){}
};

class FIFO: public Paging {
	int counter;
public:
	FIFO() {
		counter=0;
	}

	Frame* getVictimFrame() {
		if(counter==frameTable.size()) {
			counter=0;
		}
		return frameTable[counter];
	}

	void incPt() {
		counter++;
	}	
};


class SecondChance: public Paging {
	int counter;
public:
	SecondChance() {
		counter=0;
	}

	Frame* getVictimFrame() {
		if(counter==frameTable.size())
			counter=0;
		while(pt[frameTable[counter]->pageNo].refereneced==1) {
			pt[frameTable[counter]->pageNo].refereneced =0;
			counter++;
			if(counter>= frameTable.size())
				counter=0;
		}
		return frameTable[counter];
	}

	void incPt() {
		counter++;
	}	
};

class Clocking: public Paging {
	int counter;
	int type;

public:
	Clocking( int type) {
		counter=0;
		this->type = type;
	}

	Frame* getVictimFrame() {
		if(type ==0) { //On physical frames
			if(counter==frameTable.size())
				counter=0;
			while(pt[frameTable[counter]->pageNo].refereneced==1) {
				pt[frameTable[counter]->pageNo].refereneced =0;
				counter++;
				if(counter>= frameTable.size())
					counter=0;
			}
			return frameTable[counter];
		}
	else { //On virtual pages
		if(counter>63)
			counter=0;
		int flag=0;
		while(flag==0) {
			if(pt[counter].present==1) {
				if(pt[counter].refereneced==0){
					flag=1;
				}
				else 
					pt[counter].refereneced =0;
			}	
			if(flag==0) {
				counter++;
				if((counter)>63) 
					counter=0;
			}
		}
		return pt[counter].frame;	
	}
}

void incPt() {
	counter++;
}
};	

class Random: public Paging {
public:
	Random() {
	}

	Frame* getVictimFrame() {
		return frameTable[getRndmNo(frameTable.size())];
	}

	void incPt() {
	}	
};

class Aging: public Paging {
public:
	unsigned long* ageVec;
	int counter;
	Aging() {
		ageVec = new unsigned long[frameTable.size()];
		for(int i=0;i<frameTable.size();i++)
			ageVec[i]=0;
		}
	Frame* getVictimFrame() {
			int temp = 0;
			for(int i=0;i<frameTable.size();i++) {
				ageVec[i] = ageVec[i]>>1;

				if(pt[frameTable[i]->pageNo].refereneced==1) {
					ageVec[i]|=0x80000000;
					pt[frameTable[i]->pageNo].refereneced=0;
				}
				if(ageVec[i] < ageVec[temp])
					temp = i;
			}
			ageVec[temp]=0;
			return frameTable[temp];	
	}
	void incPt() {
	}
};


class VAging: public Paging {
public:
	unsigned long* ageVec;
	int counter;
	VAging() {
			ageVec = new unsigned long[64];
			for(int i=0;i<64;i++)
				ageVec[i]=0;
		}	
	Frame* getVictimFrame() {
			int temp;
			for(int i=0;i<64;i++) {
				if(pt[i].present ==1) {
					temp = i;
					break;
				}
			}
			for(int i=0;i<64;i++) {
				if(pt[i].present==1) {
					ageVec[i] = ageVec[i]>>1;
					if(pt[i].refereneced==1) {
						ageVec[i]|=0x80000000;
						pt[i].refereneced=0;
					}
					if(ageVec[i] < ageVec[temp])
						temp = i;
				}
			}
			ageVec[temp]=0;
			return pt[temp].frame;	
		}
	void incPt() {
	}
};		

class NRU: public Paging {
public:
	vector<int> class0;
	vector<int> class1;
	vector<int> class2;
	vector<int> class3;
	int counter;

	NRU() {
		counter=0;
	}

	Frame* getVictimFrame() {
		class0.clear(); class1.clear(); class2.clear(); class3.clear();
		counter++;
		for(int i=0;i<64;i++) {
			if(pt[i].present==1) {
				if(pt[i].refereneced==0 && pt[i].modified==0)
					class0.push_back(i);
				if(pt[i].refereneced==0 && pt[i].modified==1)
					class1.push_back(i);
				if(pt[i].refereneced==1 && pt[i].modified==0)
					class2.push_back(i);
				if(pt[i].refereneced==1 && pt[i].modified==1)
					class3.push_back(i);
			}
		}
		if(counter>=10) {
			for(int i=0;i<64;i++) {
				if(pt[i].present==1) 
					pt[i].refereneced=0;
			}
			counter =0;
		}
		
		Frame * frame = NULL;
		if(!class0.empty())
			frame = pt[class0[getRndmNo(class0.size())]].frame;
		else {
			if(!class1.empty())
				frame = pt[class1[getRndmNo(class1.size())]].frame;
			else {
				if(!class2.empty())
					frame = pt[class2[getRndmNo(class2.size())]].frame;
				else {
					if(!class3.empty())
						frame = pt[class3[getRndmNo(class3.size())]].frame;
				}
			}
		}
		return frame;
	}

	void incPt() {
	}	
};


int instrcount=0;
string algo = "r", options = "OPFS";
int MAX_RNDM= 0 ,rndCounter = 0,frameTableSize=32, OFlag=0,PFlag=0,FFlag=0,SFlag=0,pFlag=0,fFlag=0,aFlag=0;
int *randVals =NULL;
unsigned long long mapCount=0, unmapCount=0, inCount=0, outCout=0, zeroCount=0;
void simulation();
void printing(Frame*a, int oldPage, Instruction*b,int presentflag);
void printSumm(int counter);
void countingOps(Frame *newFrame, int oldPage, int presentflag);

void printInstructions() {
	for(list<Instruction*>::iterator iter= instructs.begin(); iter != instructs.end(); iter++)
		printf("instruct: %ld %d %d \n",(*iter)->counter,(*iter)->readWrite,(*iter)->pageNo);
}

void createInstructions(char * argv) {
	FILE *fp = NULL;
	if((fp=fopen(argv,"r"))) {
		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		char * buff =new char[size];
		rewind(fp);
		fread(buff, sizeof(char),size, fp);
		char *endLine;
		char *line = strtok_r(buff,"\n",&endLine);
		int i=0;
		while(line!=NULL) {
			if(line[0] =='#') {
				line = strtok_r(NULL,"\n",&endLine);
				continue;
			}			
			char * end;
			char * token = strtok_r(line,"\t ",&end);
			Instruction* instruct =  new Instruction();
			instruct->readWrite = atoi(token);
			token = strtok_r(NULL,"\t ",&end);
			if(atoi(token)>=0 && atoi(token)<=63)
				instruct->pageNo = atoi(token);
			else
				printf("What the heck, there is something wrong in inputfile with the instruction->pageNo %d %d\n",atoi(token),i);
			instruct->counter =i;
			i++;
			instructs.push_back(instruct);
			line = strtok_r(NULL,"\n",&endLine);
		}
		buff =NULL;	
		fclose(fp);
		instrcount=i;
		simulation();
	}
	else
		printf("\nUnable to open input-file please specific correct location\n");

}

Instruction* getInstruction() {
	Instruction* inst= instructs.front();
	return inst;
}

void setRndArry(char* argv) {
	FILE *fp = NULL;
	fp=fopen(argv,"r");
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	char * buff =new char[size];
	rewind(fp);
	fread(buff, sizeof(char),size, fp);
	char *endLine;
	char *line = strtok_r(buff,"\n",&endLine);
	MAX_RNDM = atoi(line);
	randVals = new int[MAX_RNDM];
	line = strtok_r(NULL,"\n",&endLine);
	int i =0;
	while(line!=NULL) {
		randVals[i] = atoi(line);
		i++;
		line = strtok_r(NULL,"\n",&endLine);
	}
	buff =NULL;
	fclose(fp);	
}

int getRndmNo(int no) {
	int val = randVals[rndCounter]%no ;
	if(rndCounter == MAX_RNDM-1)
		rndCounter = 0;
	else
		rndCounter++;
	return val;
}

Frame* getFrame(Paging * paging) {
	int flag=0;
	Frame *tempPt = NULL;
	for(int i=0;i<frameTableSize && flag==0;i++) {
		if(frameTable[i]->pageNo==-1) {
			flag=1;
			tempPt = frameTable[i];
		}
	}

	if(flag == 0) {
		tempPt = paging->getVictimFrame();
		paging->incPt();
	}
	return tempPt;
} 

void simulation() {
	Paging *paging =NULL;
	if(algo=="f" || algo =="F")
		paging = new FIFO();
	else
		if(algo =="s" || algo=="S")
			paging = new SecondChance();
		else
			if(algo =="c" || algo=="C")
				paging = new Clocking(0);
			else
				if(algo =="x" || algo=="X")
					paging = new Clocking(1);
				else
					if(algo =="a" || algo=="A")
						paging = new Aging();	
					else
						if(algo =="y" || algo=="Y")
							paging = new VAging();	
						else
							if(algo== "r" || algo=="R")
								paging =  new Random();	
							else
								if(algo== "n" || algo=="N")
									paging =  new NRU();		

								Frame* newFrame =NULL;
								int oldFramePage;
								Instruction* instr =getInstruction();
								while(instr!=NULL) {
									int presentflag=1;
									if(pt[instr->pageNo].present==0) {
										presentflag=0;
										newFrame = getFrame(paging);
										if(newFrame->pageNo!=-1) {
											if(pt[newFrame->pageNo].modified==1)
												pt[newFrame->pageNo].pagedOut=1;
											pt[newFrame->pageNo].present=0;	
										}
										oldFramePage= newFrame->pageNo;
										newFrame->pageNo = instr->pageNo;
										pt[instr->pageNo].frame = newFrame;
									}
									if(pt[instr->pageNo].modified==0)
										pt[instr->pageNo].modified = instr->readWrite;
									pt[instr->pageNo].refereneced = 1;
									pt[instr->pageNo].present = 1;
									printing(newFrame,oldFramePage, instr, presentflag);
									countingOps(newFrame,oldFramePage, presentflag);
									instructs.erase(instructs.begin());
									instr = getInstruction();
									if(oldFramePage!=-1) {
										pt[oldFramePage].modified=0;
									}
								}
								printSumm(instrcount);
							}

							void countingOps(Frame* newFrame, int oldPage,int presentflag) {
								if(presentflag==0) {
									if(oldPage==-1)
										zeroCount++;
									else {
										unmapCount++;
										if(pt[oldPage].modified==1)
											outCout++;
										if(pt[newFrame->pageNo].pagedOut==1)
											inCount++;
										else
											zeroCount++;	
									}
									mapCount++;
								}
							}

							void printSumm(int instruct) {
								if(PFlag==1) {
									for(int i=0;i<64;i++) {
										if(pt[i].present==1) {
											cout<<i<<":";
											if(pt[i].refereneced==1)
												cout<<"R";
											else
												cout<<"-";
											if(pt[i].modified==1)
												cout<<"M";
											else
												cout<<"-";
											if(pt[i].pagedOut==1)
												cout<<"S";
											else
												cout<<"-";
											cout<<" ";
										}
										else {
											if( pt[i].pagedOut==1)
												cout<<"# ";
											else
												cout<<"* ";
										}
									}
									cout<<"\n";
								}
								if(FFlag==1) {
									for(vector<Frame*>::iterator iter= frameTable.begin(); iter != frameTable.end(); iter++) {
										if((*iter)->pageNo==-1)
											cout<<"* ";
										else
											cout<<(*iter)->pageNo<<" ";
									}
									cout<<"\n";
								}
								if(SFlag==1) {
									cout<<"SUM "<<instruct<<" U="<<unmapCount<<" M="<<mapCount<<" I="<<inCount<<" O="<<outCout<<" Z="<<zeroCount<<" ===> "<<(instruct+zeroCount*150+(unmapCount+mapCount)*400+(outCout+inCount)*3000)<<"\n";
								}
							}


							void printing(Frame* newFrame, int oldPage, Instruction * instruct,int presentflag ) {
								if(OFlag ==1) {
									printf("==> inst: %1d %d\n", instruct->readWrite, instruct->pageNo);
									if(presentflag==0) {
										if(oldPage ==-1)
											printf("%ld: ZERO      %3d\n", instruct->counter,(int)(newFrame->frameIndex));
										else {
											printf("%ld: UNMAP %3d %3d\n", instruct->counter, oldPage, (int)(newFrame->frameIndex));
											if(pt[oldPage].modified==1) {
												printf("%ld: OUT   %3d %3d\n", instruct->counter, oldPage, (int)(newFrame->frameIndex));
											}
											if(pt[instruct->pageNo].pagedOut==1) 
												printf("%ld: IN    %3d %3d\n", instruct->counter, instruct->pageNo, (int)(newFrame->frameIndex));
											else
												printf("%ld: ZERO      %3d\n", instruct->counter, (int)(newFrame->frameIndex));
										}
										printf("%ld: MAP   %3d %3d\n", instruct->counter, instruct->pageNo,(int)(newFrame->frameIndex));
									}
								}
								if(pFlag==1) {
									for(int i=0;i<64;i++) {
										if(pt[i].present==1) {
											cout<<i<<":";
											if(pt[i].refereneced==1)
												cout<<"R";
											else
												cout<<"-";
											if(pt[i].modified==1)
												cout<<"M";
											else
												cout<<"-";
											if(pt[i].pagedOut==1)
												cout<<"S";
											else
												cout<<"-";
											cout<<" ";
										}
										else {
											if( pt[i].pagedOut==1)
												cout<<"# ";
											else
												cout<<"* ";
										}
									}
									cout<<"\n";
								}
								if(fFlag==1) {
									for(vector<Frame*>::iterator iter= frameTable.begin(); iter != frameTable.end(); iter++) {
										if((*iter)->pageNo==-1)
											cout<<"* ";
										else
											cout<<(*iter)->pageNo<<" ";
									}
									cout<<" ||";
									for(vector<Frame*>::iterator iter= frameTable.begin(); iter != frameTable.end(); iter++) {
										if((*iter)->pageNo!=-1)
											cout<<" "<<(*iter)->frameIndex<<" ";
									}
									cout<<"\n";
								}
							}

							main(int argc, char** argv) {
								int c;
								while((c = getopt(argc, argv,"o:a:f:"))!=-1) {
									switch(c) {
										case 'a':
										algo = optarg;
										break;
										case 'f':
										frameTableSize = atoi(optarg);
										Frame *frame;
										for(int i=0;i<frameTableSize;i++) {
											frame = new Frame();
											frame->frameIndex=i; 
											frameTable.push_back(frame);
										}
										break;
										case 'o':
										options = optarg;
										break;		
									}
								}
								if(argc - optind<2)
									printf("\nError need 2 fixed arguments\n");
								for(int i =0;i<options.length();i++) {
									switch(options.at(i)) {
										case 'O':
										OFlag =1;
										break;
										case 'P':
										PFlag =1;
										break;
										case 'F':
										FFlag =1;
										break;
										case 'S':
										SFlag =1;
										break;
										case 'p':
										pFlag =1;
										break;
										case 'f':
										fFlag =1;
										break;
										case 'a':
										aFlag =1;
										break;
									}
								}
								setRndArry(argv[optind+1]);
								createInstructions(argv[optind]);
							}