#include<stdio.h>

class Process {
	public:
		int at; //arrival time
		int tc; // cpu time which will go down to zero
		int ct; // cpu time
		int cb_rang; // cpu burst rang
		int ib_rang; // io burst
		int cb; // cpu burst
		int remCB; //When prempted 
		int ib; // io burst
		int staticPrior; 
		int dyPrior;
		int pid; 
		int timeInPrevState;
		int stateTS; // time it entered the prev state
		int ft; // finishing time
		int tt; // turnaround time (ft -at)
		int it; // io time
		int cw;	// cpu waiting
		int blockedTS;
		int prevStateForPrio;
		int curStateForPrio;

		Process() {
			prevStateForPrio=1;
			curStateForPrio=1;
		}
};
