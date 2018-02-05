#include<stdio.h>
#include<list>
using namespace std;
int flag=0;
class Scheduler {
public:
	list<Process*> runqueue;
	list<Process*> expiredqueue;

	Scheduler() {
	}
	virtual void addProcess(Process* process) {

	}
	virtual Process* getNextProcess() {
		Process * proc = new Process();
		return proc;
	}
	virtual void expiredProcess(Process* process) {
	}
};

class FCFS :public Scheduler {
public:
	void addProcess(Process * process) {
		runqueue.push_back(process);

	}
	Process* getNextProcess() {
		Process* proc = runqueue.front();
		runqueue.remove(runqueue.front());
		return proc;
	}

};

class LCFS :public Scheduler {
public:

	void addProcess(Process * process) {
		runqueue.push_front(process);

	}
	Process* getNextProcess() {
		Process* proc = runqueue.front();
		runqueue.remove(runqueue.front());
		return proc;
	}

};

class SJFS :public Scheduler {
public:
	void addProcess(Process * process) {
		if(runqueue.empty())
			runqueue.push_front(process);
		int flag =0;
		for(list<Process*>:: iterator iter = runqueue.begin(); iter!= runqueue.end(); iter++) {
			if(process->tc < (*iter)->tc) {
				runqueue.insert(iter,process);
				flag=1;
			}
			else
				continue;
		}
		if(flag==0)
			runqueue.push_back(process);
	}

	Process* getNextProcess() {
		Process* proc = runqueue.front();
		runqueue.remove(runqueue.front());
		return proc;
	}

};

class RR :public Scheduler {
public:
	RR(){
	}
	void addProcess(Process * process) {
		runqueue.push_back(process);
	}

	Process* getNextProcess() {
		Process* proc = runqueue.front();
		runqueue.remove(runqueue.front());
		return proc;
	}

};

class PRIO :public Scheduler {
	list<Process*> prios[3][4];
	list<Process*> *activeQ;
	list<Process*> *expiredQ;
public:
	PRIO() {	
		activeQ = &prios[0][4];
		expiredQ = &prios[1][4];

	}
	void addProcess(Process * process) {
		if(process->prevStateForPrio==3 && process->curStateForPrio==2)
			{	process->dyPrior = process->dyPrior-1;	
				process->prevStateForPrio=1;
			}
			if(process->prevStateForPrio==4 && process->curStateForPrio==2) {
				process->dyPrior =  process->staticPrior-1;	
				process->prevStateForPrio=1;
			}	

			if(process->dyPrior==-1) {
				process->dyPrior = process->staticPrior-1;
				switch(process->dyPrior) {
					case 3:
					expiredQ[3].push_back(process);
					break;
					case 2:
					expiredQ[2].push_back(process);
					break;
					case 1:
					expiredQ[1].push_back(process);
					break;
					case 0:
					expiredQ[0].push_back(process);
					break;	
				}
			}
			else {
				switch(process->dyPrior) {
					case 3:
					activeQ[3].push_back(process);
					break;
					case 2:
					activeQ[2].push_back(process);
					break;
					case 1:
					activeQ[1].push_back(process);
					break;
					case 0:
					activeQ[0].push_back(process);
					break;	
				}
			}		
		}		

		Process* getNextProcess() {
			Process * proc=NULL;
			if(activeQ[3].empty() && activeQ[2].empty() && activeQ[1].empty() && activeQ[0].empty()) {
				if(flag==0){
					activeQ = &prios[1][4];
					expiredQ = &prios[0][4];
					flag=1;
				}
				else{
					activeQ = &prios[0][4];
					expiredQ = &prios[1][4];
					flag=0;
				}
			}

			if(activeQ[3].size()!=0) {
				proc = activeQ[3].front();
				activeQ[3].remove(activeQ[3].front());
			}	
			else {
				if(activeQ[2].size()!=0) { 
					proc = activeQ[2].front();
					activeQ[2].remove(activeQ[2].front());
				}
				else {
					if(activeQ[1].size()!=0) { 
						proc = activeQ[1].front();
						activeQ[1].remove(activeQ[1].front());
					}
					else{
						if(activeQ[0].size()!=0) { 
							proc = activeQ[0].front();
							activeQ[0].remove(activeQ[0].front());
						}
					}
				}
			}	
			return proc;
		}

	};
