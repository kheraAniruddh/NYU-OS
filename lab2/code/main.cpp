#include<stdio.h>
#include<getopt.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<list>
#include<iostream>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"

using namespace std;

int QUANTUM =10000; 
int verbose = 0;
char schedArgs[20];
string schedArgus;
int rndCounter = 0;
int MAX_RNDM; 
int *randVals;
int CURRENT_TS = 0;
int SCHEDULE =0;
int ioDup=0;
int nproc=0;
double totalcpu;
double cpu;
double iototal;
list<Process*> processes;
list<Event*> events;
int ioblocks[1000000]={0};
int getRndmNo(int);
void simulation();
void printTrace(Event*);
void printSummary(double ioUtility);

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
}

int getRndmNo(int no) {
	int val = randVals[rndCounter]%no +1;
	if(rndCounter == MAX_RNDM-1)
		rndCounter = 0;
	else
		rndCounter++;
	return val;
}

void printEvents() {
	for(list<Event*>::iterator iter= events.begin(); iter != events.end(); iter++)
		printf("events %d %d \n",CURRENT_TS,(*iter)->process->pid);
}


void createProcess(char *argv) {
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
			char * end;
			char * token = strtok_r(line,"\t ",&end);
			Event *event = new Event();
			Process  *process = new Process();
			process->at = atoi(token);
			event->timeStamp =atoi(token);
			token = strtok_r(NULL,"\t ",&end);
			process->tc = atoi(token);
			process->ct= atoi(token);
			token = strtok_r(NULL,"\t ",&end);
			process->cb_rang = atoi(token);
			token = strtok_r(NULL,"\t ",&end);
			process->ib_rang = atoi(token);
			process->staticPrior = getRndmNo(4);
			process->dyPrior = process->staticPrior-1;
			process->pid = i;
			i++;
			event->oldState = CREATED;
			event->process = process;
			event->newState = READY;
			processes.push_back(process);
			events.push_back(event);
			line = strtok_r(NULL,"\n",&endLine);
		}
		simulation();	
	}
	else
		printf("\nUnable to open input-file please specific correct location\n");		
}

Event * getEvent() {
	if(events.front()!=NULL)
		return events.front();
	return NULL;

}

void putEvent(Event *event) {
	if(events.empty())
		events.push_back(event);
	else {
		char flag = 0;
		for(list<Event*>::iterator iter= events.begin(); iter != events.end(); iter++) {
			if(event->timeStamp >= (*iter)->timeStamp)
				continue;				
			else {
				flag=1;
				events.insert(iter,event);
				break;
			}
		}
		if(flag==0)
			events.push_back(event);
	}
}


void simulation() {
	int last;
	Event* event;
	Scheduler * sch;
	Process * CURRENT_RUNNG_PROC = NULL;
	schedArgus = string(schedArgs);
	if(schedArgus.length()==1){
		if((schedArgus.substr(0,1)=="F")||(schedArgus.substr(0,1)=="f"))
			sch = new FCFS();
		if((schedArgus.substr(0,1)=="L")||(schedArgus.substr(0,1)=="l"))
			sch = new LCFS();
		if((schedArgus.substr(0,1)=="S")||(schedArgus.substr(0,1)=="s"))
			sch = new SJFS();
	}
	else {
		if((schedArgus.substr(0,1)=="R")||(schedArgus.substr(0,1)=="r")) {
			QUANTUM = atoi(schedArgus.substr(1,schedArgus.length()).c_str());
			sch = new RR();
		}
		if((schedArgus.substr(0,1)=="P")||(schedArgus.substr(0,1)=="p")) {
			QUANTUM = atoi(schedArgus.substr(1,schedArgus.length()).c_str());
			sch = new PRIO();
		}
	}
	Event * evt;
	while((event = getEvent())!=NULL) {
		CURRENT_TS = event->timeStamp;
		event->process->timeInPrevState = CURRENT_TS - event->process->stateTS;
		switch(event->getTransition(event->oldState,event->newState)) {
		
			case 1:
			event->process->timeInPrevState = 0;
			case 4:
			event->process->prevStateForPrio = event->oldState;
			event->process->curStateForPrio = event->newState;
			sch->addProcess(event->process);
			SCHEDULE = 1; 
			break;	
	
			case 2:
			event->process->cw+= CURRENT_TS - event->process->stateTS;
			if(event->process->remCB <= 0)
				event->process->cb = getRndmNo(event->process->cb_rang);
			else {
				event->process->cb  = event->process->remCB;
				event->process->remCB = 0;
			}
			if(event->process->tc < event->process->cb)
				event->process->cb = event->process->tc;
			evt = new Event();
			evt->process = event->process;
			if(event->process->cb > QUANTUM) {
				event->process->remCB = event->process->cb - QUANTUM;
				event->process->cb = QUANTUM;
				evt->oldState = RUNNG;
				evt->newState = READY;
			}
			else {
				evt->oldState = RUNNG;
				evt->newState = BLOCK;
			}
			CURRENT_RUNNG_PROC = event->process;
			evt->timeStamp = CURRENT_TS+event->process->cb;
			putEvent(evt);
			break;

			case 3:
			event->process->tc -= event->process->cb;
			if(event->process->tc>0) {
				event->process->ib = getRndmNo(event->process->ib_rang);
				for(int i=CURRENT_TS+1;i<=(CURRENT_TS+ event->process->ib);i++) {
					ioblocks[i]=1;
				}	
				event->process->it+=event->process->ib;
				evt = new Event();
				evt->timeStamp = CURRENT_TS + event->process->ib;
				evt->process = event->process;
				evt->oldState = BLOCK;
				evt->newState = READY;
				putEvent(evt);
			}
			else {
				event->process->ft = CURRENT_TS;
				event->process->tt = event->process->ft-event->process->at;
				last = CURRENT_TS;
			}
			CURRENT_RUNNG_PROC = NULL;
			SCHEDULE = 1;
			break;

			case 5:
			event->process->tc -= event->process->cb;
			event->process->prevStateForPrio =  event->oldState;
			event->process->curStateForPrio = event->newState;
			sch->addProcess(event->process);
			CURRENT_RUNNG_PROC = NULL;
			SCHEDULE = 1;
			break;	
		}
		printTrace(event);
		event->process->stateTS = CURRENT_TS;
		events.remove(event);
		if(SCHEDULE) {
			Event* checkEnv= getEvent();
			if(checkEnv!=NULL) {
				if(checkEnv->timeStamp == CURRENT_TS ){
					continue;
				}	
			}
			SCHEDULE = 0;
			if(CURRENT_RUNNG_PROC == NULL) {
				CURRENT_RUNNG_PROC = sch->getNextProcess();
				if(CURRENT_RUNNG_PROC == NULL)
					continue;
				evt = new Event();
				evt->process = CURRENT_RUNNG_PROC;
				evt->timeStamp = CURRENT_TS;
				evt->newState = RUNNG;
				evt->oldState = READY;
				putEvent(evt);
			}

		}
	}
	double ioUtility=0;
	for(int i=0;i<last;i++)
		if(ioblocks[i]==1)
			ioUtility++;
		printSummary(ioUtility);
	}

	void printSummary(double ioUtility) {
		if((schedArgus.substr(0,1)=="F")||(schedArgus.substr(0,1)=="f"))
			printf("FCFS\n");
		if((schedArgus.substr(0,1)=="L")||(schedArgus.substr(0,1)=="l"))
			printf("LCFS\n");
		if((schedArgus.substr(0,1)=="S")||(schedArgus.substr(0,1)=="s"))
			printf("SJF\n");
		if((schedArgus.substr(0,1)=="R")||(schedArgus.substr(0,1)=="r"))
			printf("RR %d\n",QUANTUM);
		if((schedArgus.substr(0,1)=="P")||(schedArgus.substr(0,1)=="p"))
			printf("PRIO %d\n",QUANTUM);
		int noOfProcs =0;
		double ttt=0;
		double totalIdle = 0;
		int finalfin= 0;

		for(list<Process*>:: iterator iter = processes.begin();iter!=processes.end();iter++) {
			noOfProcs++;
			totalcpu+= (*iter)->ct;
			iototal+=(*iter)->it;
			ttt += (*iter)->tt;
			totalIdle += (*iter)->cw;
			if(finalfin<(*iter)->ft)
				finalfin = (*iter)->ft;
			printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",(*iter)->pid, (*iter)->at, (*iter)->ct, (*iter)->cb_rang, (*iter)->ib_rang, (*iter)->staticPrior, (*iter)->ft, (*iter)->tt, (*iter)->it, (*iter)->cw);
		}
		cpu = (totalcpu/finalfin)*100;
		double ioUtil = (ioUtility/finalfin)*100;
		double denom= finalfin;
		double throughput = (noOfProcs/denom)*100;

		printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",finalfin, cpu, ioUtil, (ttt/noOfProcs), (totalIdle/noOfProcs), throughput);
	}

	void printTrace(Event * event) {
		if(verbose == 1) {
			if(event->process->tc<=0)
				printf("%d %d %d: Done\n",CURRENT_TS, event->process->pid,event->process->timeInPrevState);
			else { 
				if(event->getTransition(event->oldState,event->newState)==2 || event->getTransition(event->oldState,event->newState)==5)
					printf("%d %d %d: %s -> %s cb=%d rem=%d prio=%d\n",CURRENT_TS, event->process->pid ,event->process->timeInPrevState, states[event->oldState],states[event->newState],event->process->cb,event->process->tc,event->process->dyPrior);
				if(event->getTransition(event->oldState,event->newState)==3)
					printf("%d %d %d: %s -> %s ib=%d rem=%d\n",CURRENT_TS, event->process->pid ,event->process->timeInPrevState, states[event->oldState],states[event->newState],event->process->ib,event->process->tc);
				if(event->getTransition(event->oldState,event->newState)==1 || event->getTransition(event->oldState,event->newState)==4)
					printf("%d %d %d: %s -> %s\n",CURRENT_TS, event->process->pid ,event->process->timeInPrevState, states[event->oldState],states[event->newState]); 
			}		
		}
	}


	main(int argc, char** argv) {
		int c;
		while((c = getopt(argc, argv,"s:v"))!=-1) {
			switch(c) {
				case 'v':
				verbose = 1;
				break;
				case 's':
				strcpy(schedArgs,optarg);
				break;	
			}
		}
		if(argc - optind<2)
			printf("\nError need 2 fixed arguments\n");
		setRndArry(argv[optind+1]);
		createProcess(argv[optind]);
	}
