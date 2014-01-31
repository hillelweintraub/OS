// Hillel Weintraub
// 11/27/13
// Operating Systems-Problem 8
// 
// syscost - measure the cost of a system call

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

void myfunc(){;}

int main(int argc, char const *argv[]){
	struct timespec begin,end;
	int numiters=1e8,i;
	if(clock_gettime(CLOCK_REALTIME,&begin)<0){
		perror("clock_gettime");
		return -1;
	}
	for(i=0;i<numiters;i++){
		;
	}
	if(clock_gettime(CLOCK_REALTIME,&end)<0){
		perror("clock_gettime");
		return -1;
	}
	double looptime= ((double)end.tv_sec*1.0e9+(double)end.tv_nsec-(double)begin.tv_sec*1.0e9-(double)begin.tv_nsec)/numiters;
	fprintf(stderr,"loop time: %f nanoseconds\n",looptime);

	if(clock_gettime(CLOCK_REALTIME,&begin)<0){
		perror("clock_gettime");
		return -1;
	}
	for(i=0;i<numiters;i++){
		myfunc();
	}
	if(clock_gettime(CLOCK_REALTIME,&end)<0){
		perror("clock_gettime");
		return -1;
	}
	double functime= ((double)end.tv_sec*1.0e9+(double)end.tv_nsec-(double)begin.tv_sec*1.0e9-(double)begin.tv_nsec)/numiters - looptime;
	fprintf(stderr,"function call time: %f nanoseconds\n",functime);

	if(clock_gettime(CLOCK_REALTIME,&begin)<0){
		perror("clock_gettime");
		return -1;
	}
	for(i=0;i<numiters;i++){
		getuid();
	}
	if(clock_gettime(CLOCK_REALTIME,&end)<0){
		perror("clock_gettime");
		return -1;
	}
	double systime= ((double)end.tv_sec*1.0e9+(double)end.tv_nsec-(double)begin.tv_sec*1.0e9-(double)begin.tv_nsec)/numiters - looptime - functime;
	fprintf(stderr,"system call time: %f nanoseconds\n",systime);

	fprintf(stderr,"A system call is approximately %f times more expensive than a function call\n",systime/functime);
	return 0;
}