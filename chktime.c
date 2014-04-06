#include "global.h"

struct timeval start;
struct timeval end;
long timevaldiff(struct timeval *starttime, struct timeval *finishtime)
{
    long msec;
    msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
    msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
    return msec;
}

void time_start(){
    gettimeofday(&start, NULL);
}

void time_end(){
    gettimeofday(&end, NULL);
	printf("It took %ld msec\n", timevaldiff(&start, &end));
}
