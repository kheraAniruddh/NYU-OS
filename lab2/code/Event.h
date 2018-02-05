#include<stdio.h>

enum STATE {
	CREATED=1, READY, RUNNG, BLOCK
};


static  char const* states[] ={"","CREATED","READY","RUNNG","BLOCK"};

class Event {
	public:
	int timeStamp;
	Process * process;
	STATE oldState;
	STATE newState;

	int getTransition(STATE oldState, STATE newState) {
		if(oldState==CREATED && newState==READY)
			return 1;
		if(oldState==READY && newState==RUNNG)
			return 2;
		if(oldState==RUNNG && newState==BLOCK)
			return 3;
		if(oldState==BLOCK && newState==READY)
			return 4;
		if(oldState==RUNNG && newState==READY)
			return 5;
	}
};