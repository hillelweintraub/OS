// Hillel Weintraub
// 11/20/13
// Operating Systems-Problem 7
// 
// sem.c - The implementation of the semaphore module

#include "sem.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <errno.h>


void usr1_handler(int sn){}

void sem_init(struct sem *s, int count){
	s->lock=0;
	s->count=count;
	sigfillset(&s->fullset);
	int i;
	for(i=0;i<N_PROC;i++)
		s->pids[i]=0;
	if(signal(SIGUSR1,usr1_handler)==SIG_ERR){
		perror("Error setting handler for SIGUSR1");
		exit(-1);
	}
}

int sem_try(struct sem *s){
	sigset_t oldset;
	sigprocmask(SIG_BLOCK,&s->fullset,&oldset);
	while(tas(&s->lock))
		;
	if(!s->count){ //operation would block
		s->lock=0;
		sigprocmask(SIG_SETMASK,&oldset,NULL);
		return 0;
	}else{
		s->count--;
		s->lock=0;
		sigprocmask(SIG_SETMASK,&oldset,NULL);
		return 1;
	}
}

void sem_wait(struct sem *s){
	sigset_t oldset,suspendset;
	suspendset=s->fullset;
	sigdelset(&suspendset,SIGUSR1);
	sigprocmask(SIG_BLOCK,&s->fullset,&oldset);
	for(;;){
		while(tas(&s->lock))
			;
		if(!s->count){
			s->lock=0;
			if(my_procnum<0||my_procnum>N_PROC){
				fprintf(stderr,"Error procnum is not in the valid range (0-%d)\n",N_PROC);
				exit(-1);
			}
			s->pids[my_procnum]=getpid();
			sigsuspend(&suspendset);
			continue;
		}
		break;
	}
	s->count--;
	s->lock=0;
	sigprocmask(SIG_SETMASK,&oldset,NULL);
}

void sem_inc(struct sem *s){
	sigset_t oldset;
	sigprocmask(SIG_BLOCK,&s->fullset,&oldset);
	while(tas(&s->lock))
		;
	s->count++;
	int i;
	for(i=0;i<N_PROC;i++){
		if(s->pids[i]){
			if(kill(s->pids[i],SIGUSR1)==-1){
				perror("kill");
				exit(-1);
			}
			s->pids[i]=0;
		}
	}
	s->lock=0;
	sigprocmask(SIG_SETMASK,&oldset,NULL);
}


