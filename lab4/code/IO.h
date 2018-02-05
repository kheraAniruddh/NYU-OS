#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<list>

using namespace std;
class IO {
public:
	IO() {finishTime =-1; issueTime=-1;}
	int startTime;
	int finishTime;
	int issueTime;
	int track;
	int id;
};

enum STATE {
	CREATED=1, ADD, ISSUE, FINISH
};


static  char const* states[] ={"","CREATED","ADD","ISSUE","FINISH"};

class Event {
public:
	int timeStamp;
	IO * io;
	STATE oldState;
	STATE newState;

	int getTransition(STATE oldState, STATE newState) {
		if(oldState==CREATED && newState==ADD)
			return 1;
		if(oldState==ADD && newState==ISSUE)
			return 2;
		if(oldState==ISSUE && newState==FINISH)
			return 3;
	}
};

class Scheduler {
public:
	list<IO*> ioqueue;
	int currentTrack;
	int issueFlag;

	Scheduler() {
	}
	virtual void addIO(IO* io) {

	}
	virtual IO* getNextIO() {
		IO * io = new IO();
		return io;
	}

	virtual void finishSignal() {
	}
};

class FIFO :public Scheduler {
public:
	void addIO(IO * io) {
		ioqueue.push_back(io);

	}
	IO* getNextIO() {
		IO* io = ioqueue.front();
		ioqueue.remove(ioqueue.front());
		return io;
	}

};

class SSTF :public Scheduler {
public:
	SSTF() {
		currentTrack =0;
	}
	void addIO(IO * io) {
		ioqueue.push_back(io);
	}

	IO* getNextIO() {
		IO * io = ioqueue.front();
		for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
			if(abs(currentTrack-io->track)> abs(currentTrack-(*iter)->track)) 
				io = (*iter);
		}
		for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
			if(io == (*iter)) {
				ioqueue.erase(iter);
				break;
			}
		}
		if(io!=NULL)
			currentTrack = io->track;	
		return io;
	}


};

class SCAN :public Scheduler {
public:
	int direction;
	SCAN() {
		currentTrack =0;
		direction=1;
	}
	void addIO(IO * io) {
		ioqueue.push_back(io);
	}

	IO* getNextIO() {
		IO * io;
		if(direction==1)
			io =UP();
		else
			io = DOWN();
		if(io!=NULL){
			for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
				if(io == (*iter)) {
					ioqueue.erase(iter);
					break;
				}
			}
			currentTrack = io->track;	
		}
		return io;
	}

	IO* UP() {
		int flag=0;
		IO * io;
		if(io==NULL)
			return io;
		for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
			if((*iter)->track>=currentTrack){
				io =*iter;
				flag=1;
				break;
			}
		}
		if(flag==1) {
			for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
				if(((io->track)-currentTrack)> (((*iter)->track)-currentTrack) && (*iter)->track>=currentTrack) 
					io = (*iter);
			}
			return io;
		}
		else{
			direction=0;
			io=NULL;
			DOWN();
		}
	}

	IO* DOWN() {
		int flag=0;
		IO* io;
		if(io==NULL)
			return io;
		for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
			if((*iter)->track<=currentTrack){
				io =*iter;
				flag=1;
				break;
			}
		}
		if(flag==1) {
			for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
				if((currentTrack-io->track > currentTrack-(*iter)->track) && (*iter)->track<=currentTrack) 
					io = (*iter);
			}
			return io;
		}
		else{
			direction=1;
			io=NULL;
			UP();
		}
	}
};

class CSCAN :public Scheduler {
public:
	CSCAN() {
		currentTrack =0;
	}
	void addIO(IO * io) {
		ioqueue.push_back(io);
	}

	IO* getNextIO() {
		IO * io;
		io =UP();
		if(io!=NULL){
			for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
				if(io == (*iter)) {
					ioqueue.erase(iter);
					break;
				}
			}
			currentTrack = io->track;	
		}
		return io;
	}

	IO* UP() {
		int flag=0;
		IO * io;
		for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
			if((*iter)->track>=currentTrack){
				io =*iter;
				flag=1;
				break;
			}
		}
		if(flag==1) {
			for(list<IO*>::iterator iter= ioqueue.begin(); iter != ioqueue.end(); iter++) {
				if((((io->track)-currentTrack)> (((*iter)->track)-currentTrack)) && (*iter)->track>=currentTrack) 
					io = (*iter);
			}
			return io;
		}
		else{
			currentTrack =0;
			if(io!=NULL)
				io= UP();
			return io;
		}
	}
};

class FSCAN :public Scheduler {
public:
	list<IO*> queueptr[2];
	list<IO*> *Q1;
	list<IO*> *Q2;
	int swap;
	int direction;
	FSCAN() {
		currentTrack =0;
		Q1 = &queueptr[0];
		Q2 = &queueptr[1];
		issueFlag=0;
		swap=0;
		direction=1;
	}
	void addIO(IO * io) {
		if(issueFlag==0) {
			Q1->push_back(io);
		}
		else {
			Q2->push_back(io);
		}
	}

	IO* getNextIO() {
		issueFlag=1;
		if(Q1->empty()){
			if(swap==0) {
				Q1 = &queueptr[1];
				Q2 = &queueptr[0];
				swap=1;
				direction=1;
			}
			else {
				Q1 = &queueptr[0];
				Q2 = &queueptr[1];
				swap=0;
				direction=1;
			}
		}

		IO * io;
		if(direction==1)
			io =UP();
		else
			io = DOWN();
		if(io!=NULL){
			for(list<IO*>::iterator iter= Q1->begin(); iter != Q1->end(); iter++) {
				if(io == (*iter)) {
					Q1->erase(iter);
					break;
				}
			}
			currentTrack = io->track;	
		}
		return io;
	}

		virtual void finishSignal() {
			issueFlag=0;
	}

	IO* UP() {
		int dflag=0;
		IO * io;
		if(io==NULL)
			return io;
		for(list<IO*>::iterator iter= Q1->begin(); iter != Q1->end(); iter++) {
			if((*iter)->track>=currentTrack){
				io =*iter;
				dflag=1;
				break;
			}
		}
		if(dflag==1) {
			for(list<IO*>::iterator iter= Q1->begin(); iter != Q1->end(); iter++) {
				if(((io->track)-currentTrack)> (((*iter)->track)-currentTrack) && (*iter)->track>=currentTrack) 
					io = (*iter);
			}
			return io;
		}
		else{
			direction=0;
			io=NULL;
			DOWN();
		}
	}

	IO* DOWN() {
		int dflag=0;
		IO* io;
		if(io==NULL)
			return io;
		for(list<IO*>::iterator iter= Q1->begin(); iter != Q1->end(); iter++) {
			if((*iter)->track<=currentTrack){
				io =*iter;
				dflag=1;
				break;
			}
		}
		if(dflag==1) {
			for(list<IO*>::iterator iter= Q1->begin(); iter != Q1->end(); iter++) {
				if((currentTrack-io->track > currentTrack-(*iter)->track) && (*iter)->track<=currentTrack) 
					io = (*iter);
			}
			return io;
		}
		else{
			direction=1;
			io=NULL;
			UP();
		}
	}
};