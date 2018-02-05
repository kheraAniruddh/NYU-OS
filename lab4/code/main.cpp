#include<stdio.h>
#include<getopt.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<list>
#include<iostream>
#include "IO.h"

using namespace std;

list<Event*> events;
list<IO*> iolist;
string algo;
int verbose=0;
int prevTrack=0;
void simulation();
void printTrace(Event* event);
void printing(int total_time, int tot_movement);

void printEvents() {
	for(list<Event*>::iterator iter= events.begin(); iter != events.end(); iter++)
		printf("IO: %d %d %d \n",(*iter)->io->id,(*iter)->timeStamp,(*iter)->io->track);
}

void createIOs(char * argv) {
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
		IO* io;
		Event* event;
		while(line!=NULL) {
			if(line[0] =='#') {
				line = strtok_r(NULL,"\n",&endLine);
				continue;
			}			
			char * end;
			char * token = strtok_r(line,"\t ",&end);
			io =  new IO();
			event =  new Event();
			io->startTime = atoi(token);
			event->timeStamp = atoi(token);
			token = strtok_r(NULL,"\t ",&end);
			io->track = atoi(token);
			io->id =i;
			iolist.push_back(io);
			event->io = io;
			event->oldState = CREATED;
			event->newState = ADD;
			events.push_back(event);
			i++;
			line = strtok_r(NULL,"\n",&endLine);
		}
		buff =NULL;	
		fclose(fp);
		simulation();
	//	printEvents();
	}
	else
		printf("\nUnable to open input-file please specific correct location\n");

}

Event* getEvent() {
	Event* event= events.front();
	return event;
}
void addEvent(Event * event) {
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
	Event* event = getEvent();
	Event * evt;
	int CURRENT_TS;
	int SCHEDULE =0;
	int total_time=0;
	int tot_movement =0;
	IO * CURRENT_IO= NULL;
	Scheduler* sched;
	if(algo=="i")
		sched = new FIFO();
	else
	if(algo=="j")
		sched = new SSTF();
	else
	if(algo=="s")
		sched = new SCAN();
	else
	if(algo=="c")
		sched = new CSCAN();
	else
	if(algo=="f")
		sched = new FSCAN();
	int count=0;

	while((event = getEvent())!=NULL) {
		CURRENT_TS = event->timeStamp;
		switch(event->getTransition(event->oldState, event->newState)) {
			case 1: //ADD
			sched->addIO(event->io);
			SCHEDULE =1;
			break;
			case 2: //ISSUE
			CURRENT_IO = event->io;
			event->io->issueTime = CURRENT_TS; 
			evt = new Event();
			evt->oldState = ISSUE;
			evt->newState = FINISH;
			evt->io = event->io;
			evt->timeStamp = CURRENT_TS + abs(event->io->track- prevTrack);
			tot_movement+= abs(event->io->track- prevTrack);
			addEvent(evt);
			break;
			case 3: //FINISH
			event->io->finishTime = CURRENT_TS;
			prevTrack= event->io->track;
			CURRENT_IO =NULL;
			sched->finishSignal();
			SCHEDULE=1;
			break;
		}
			printTrace(event);
			events.remove(event);
			if(SCHEDULE) {
				Event * newEvent = getEvent();
				if(newEvent!=NULL) {
					if(newEvent->timeStamp == CURRENT_TS)
						continue;
				}
				SCHEDULE = 0;
				if(CURRENT_IO == NULL) {
					CURRENT_IO = sched->getNextIO();
					if(CURRENT_IO == NULL)
						continue;
					evt = new Event();
					evt->io = CURRENT_IO;
					evt->timeStamp = CURRENT_TS;
					evt->newState = ISSUE;
					evt->oldState = ADD;
					addEvent(evt);
				}

			}
		}
		printing(CURRENT_TS, tot_movement);

}

void printTrace(Event* event) {
	if(verbose) {
		if(event->newState == ADD)
			cout<<event->timeStamp<<":\t"<<event->io->id<<" add"<<" "<<event->io->track<<endl;
		else
		if(event->newState== ISSUE)
			cout<<event->timeStamp<<":\t"<<event->io->id<<" issue"<<" "<<event->io->track<<" "<<prevTrack<<endl;
		else
		if(event->newState== FINISH)
			cout<<event->timeStamp<<":\t"<<event->io->id<<" finish"<<" "<<event->io->finishTime-event->io->startTime<<endl;
	}
}

void printing(int total_time, int tot_movement) {
	if(verbose) {
		cout<<"IOREQS INFO\n";
		for(list<IO*>::iterator iter= iolist.begin(); iter != iolist.end(); iter++)
			printf("%d:\t%d\t%d\t%d\n",(*iter)->id,(*iter)->startTime,(*iter)->issueTime, (*iter)->finishTime);
	}
	double total_turnaround =0;
	double total_wt = 0;
	int count =0;
	int max_waittime=0;
	for(list<IO*>::iterator iter= iolist.begin(); iter != iolist.end(); iter++) {
		total_turnaround+= (*iter)->finishTime -(*iter)->startTime;
		if(max_waittime<(*iter)->issueTime - (*iter)->startTime)
			max_waittime = (*iter)->issueTime - (*iter)->startTime;
		total_wt+=(*iter)->issueTime - (*iter)->startTime;
		count++;
	}
	printf("SUM: %d %d %.2lf %.2lf %d\n",total_time, tot_movement, total_turnaround/count, total_wt/count, max_waittime);
}

main(int argc, char** argv) {
	int c;
	while((c = getopt(argc, argv,"s:v"))!=-1) {
		switch(c) {
			case 's':
			algo = optarg;
			break;
			case 'v':
			verbose=1;
			break;		
		}
	}
	if(argc - optind<1)
		printf("\nError need 1 fixed arguments\n");
	createIOs(argv[optind]);
}
