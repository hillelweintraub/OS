// Hillel Weintraub
// 11/20/13
// Operating Systems-Problem 7
// 
// fifo.h - header file for fifo module

#ifndef FIFO_H
#define FIFO_H 
#include "sem.h"
#define MYFIFO_BUFSIZE 4096


//The fifo data structure
struct fifo{
	unsigned long buf[MYFIFO_BUFSIZE];
	int next_write,next_read;
	struct sem full,empty;
	struct sem mutex; //mutex on buf,next_write and next_read 
};

//Initialize the shared memory FIFO *f
void fifo_init(struct fifo *f);

//Enques the data word d into the FIFO,blocking
//unless and until the FIFO has room to accept it
void fifo_wr(struct fifo *f,unsigned long d);

//Deque the next data word from the FIFO and return it.
//Block unless and until there are available words queued in the FIFO.
unsigned long fifo_rd(struct fifo *f);
#endif