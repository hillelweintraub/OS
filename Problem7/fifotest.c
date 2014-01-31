// Hillel Weintraub
// 11/20/13
// Operating Systems-Problem 7
// 
// fifotest.c - A program that tests the FIFO implementation

#include "sem.h"
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

int my_procnum;
int main(int argc, char const *argv[]){
	struct fifo f, *fp;
	fifo_init(&f);
	if((fp=(struct fifo *)mmap(NULL,sizeof f,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0))==MAP_FAILED){
		perror("Error using mmap to establish a struct fifo in shared memory");
		return -1;
	}
	memcpy(fp,&f,sizeof f);
	unsigned long d;
	int i,procnum,seqlength=300;
	for(procnum=1;procnum<N_PROC;procnum++){
		switch(fork()){
			case -1:
				perror("fork failed");
				return -1;
				break;
			case 0: // child (writer)
				my_procnum=procnum;
				for(i=0;i<seqlength;i++){
					d=my_procnum|(i<<6); //bitwise word with proc id in 1st six bits and seq# in the remaining bits
					fifo_wr(fp,d);
				}
				return 0;
				break;
				default: // parent (reader)
					break;
		}
	}
	my_procnum=0;
	for(i=0;i<seqlength*(N_PROC-1);i++){
		d=fifo_rd(fp);
		procnum=d&0x3f;
		d=d>>6; //sequence#
		fprintf(stdout,"Proccess#:%d, Sequence#:%lu, absolute#: %d\n",procnum,d,i);
	}
	for(procnum=1;procnum<N_PROC;procnum++){
		int status;
		if(wait(&status)==-1){
			perror("Wait failed");
			return -1;
		}
	}
	if(munmap(fp,sizeof f)==-1){
		perror("Error calling munmap");
		return -1;
	}
	return 0;
}