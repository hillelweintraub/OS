// Hillel Weintraub
// 11/20/13
// Operating Systems-Problem 7
// 
// sem.h - header file for semaphore module

#ifndef SEM_H
#define SEM_H 
#include <signal.h>
#define N_PROC 64 //the max number of processes this implementation handles

extern int my_procnum;

//Atomic test and set instruction
//returns:
//			0: if the lock has been acquired
//			1: if the lock could not be acquired
int tas(volatile char *lock); 

//signal handler on SIGUSR1 to allow for waking up after a sigsuspend
void usr1_handler(int sn);

//The semaphore data structure
struct sem{
	char lock;
	int count;
	int pids[N_PROC];
	sigset_t fullset;
};

 //Initializes the semaphore *s with the initial count 
//and initializes any underlying data structures
void sem_init(struct sem *s, int count);


//Attempt to perform "P" operation (atomically decrement the semaphore)
//returns:
//			0: if this operation would block
//			1: on success
int sem_try(struct sem *s);

//Performs the "P" operation blocking until successful
void sem_wait(struct sem *s);

//Performs the "V" operation and wakes any other processors that were sleeping on this semaphore
void sem_inc(struct sem *s);

#endif