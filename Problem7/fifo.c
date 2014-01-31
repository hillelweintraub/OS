// Hillel Weintraub
// 11/20/13
// Operating Systems-Problem 7
// 
// fifo.c - Implementation of the  fifo module

#include "fifo.h"

void fifo_init(struct fifo *f){
	f->next_write=0;
	f->next_read=0;
	sem_init(&f->empty,0);
	sem_init(&f->full,MYFIFO_BUFSIZE);
	sem_init(&f->mutex,1); 
}

void fifo_wr(struct fifo *f,unsigned long d){
	sem_wait(&f->full);
	while(!sem_try(&f->mutex))
		;
	f->buf[f->next_write++]=d;
	f->next_write%=MYFIFO_BUFSIZE;
	sem_inc(&f->mutex);
	sem_inc(&f->empty);
}

unsigned long fifo_rd(struct fifo *f){
	unsigned long d;
	sem_wait(&f->empty);
	while(!sem_try(&f->mutex))
		;
	d=f->buf[f->next_read++];
	f->next_read%=MYFIFO_BUFSIZE;
	sem_inc(&f->mutex);
	sem_inc(&f->full);
	return d;
}
