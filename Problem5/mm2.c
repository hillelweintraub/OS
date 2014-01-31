// Hillel Weintraub
// 11/6/13
// Operating Systems-Problem 5
// 
// mm2 - test program to answer questions about the virtual memory system

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <setjmp.h>
#define FILESIZE 8195

void sighandler(int sn);
char *fileDump(char *fname,int fd,int offset,int req);
void memDump(char *map,int offset,int n);
void createFile(char *fname,int fsize);
char const **g_argv;
char *fname;
jmp_buf jb;
int main(int argc, char const **argv){
	g_argv=argv;
	if(argc<2) fprintf(stderr,
		"Error: Improper usage. Supply a test question argument [A-F]\n");
	char *buf,*buf2,c;
	char *map;	
	int fd,i,expandsize,numbytes,dataremains,fsize,newfsize,sn;
	struct stat st;
	switch(argv[1][0]){
		case 'A':
			fname="testfile";
			createFile(fname,FILESIZE);
			if ((fd=open(fname,O_RDWR))<0){
				fprintf(stderr,"Can’t open file %s for reading and writing",fname);
				perror("");
				return -1;
			}
			fprintf(stdout,"About to map file with MAP_SHARED and PROT_READ\n");
			if((map=(char *)mmap(NULL,FILESIZE,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED){
				fprintf(stderr,"Error using mmap with PROT_READ and MAP_SHARED on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			for(i=1;i<32;i++){
				if(i==9 || i==19) continue;
				if(signal(i,sighandler)==SIG_ERR){
					fprintf(stderr,"Error setting signal handler for signal #%d: %s\n",i,strerror(errno));
					return -1;
				}
			}
			fprintf(stdout,"About to write to the mapped region\n");
			map[0]='a';
			break;
		case 'B':
			fname="testfile";
			createFile(fname,FILESIZE);
			if ((fd=open(fname,O_RDWR))<0){
				fprintf(stderr,"Can’t open file %s for reading and writing",fname);
				perror("");
				return -1;
			}
			fprintf(stdout,"About to map file with MAP_SHARED\n");
			if((map=(char *)mmap(NULL,FILESIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED){
				fprintf(stderr,"Error using mmap with PROT_READ, PROT_WRITE and MAP_SHARED on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			numbytes=4;
			buf=fileDump(fname,fd,0,numbytes);
			fprintf(stdout,"About to write %d bytes to the region at offset 0\n",numbytes);
			for(i=0;i<numbytes;i++){
				map[i]=buf[i]+1;
			}
			memDump(map,0,numbytes);
			buf2=fileDump(fname,fd,0,numbytes);
			if(strncmp(map,buf2,numbytes)==0){
				fprintf(stdout,"In response to question B: Yes the update is immediately visible in the file using read(2)\n");
			}else{
				fprintf(stdout,"In response to question B: No the update is not immediately visible in the file using read(2)\n");
			}
			free(buf);
			free(buf2);
			if(close(fd)==-1){
				fprintf(stderr,"Error closing file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(unlink(fname)==-1){
				fprintf(stderr,"Error unlinking file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(munmap(map,FILESIZE)==-1){
				perror("Error calling munmap");
				return -1;
			}
			break;
		case 'C':
			fname="testfile";
			createFile(fname,FILESIZE);
			if ((fd=open(fname,O_RDWR))<0){
				fprintf(stderr,"Can’t open file %s for reading and writing",fname);
				perror("");
				return -1;
			}
			fprintf(stdout,"About to map file with MAP_PRIVATE\n");
			if((map=(char *)mmap(NULL,FILESIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0))==MAP_FAILED){
				fprintf(stderr,"Error using mmap with PROT_READ, PROT_WRITE and MAP_PRIVATE on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			numbytes=4;
			buf=fileDump(fname,fd,0,numbytes);
			fprintf(stdout,"About to write %d bytes to the region at offset 0\n",numbytes);
			for(i=0;i<numbytes;i++){
				map[i]=buf[i]+1;
			}
			memDump(map,0,numbytes);
			buf2=fileDump(fname,fd,0,numbytes);
			if(strncmp(map,buf2,numbytes)==0){
				fprintf(stdout,"In response to question C: Yes the update is immediately visible in the file using read(2)\n");
			}else{
				fprintf(stdout,"In response to question C: No the update is not immediately visible in the file using read(2)\n");
			}
			free(buf);
			free(buf2);
			if(close(fd)==-1){
				fprintf(stderr,"Error closing file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(unlink(fname)==-1){
				fprintf(stderr,"Error unlinking file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(munmap(map,FILESIZE)==-1){
				perror("Error calling munmap");
				return -1;
			}
			break;
		case 'D':
			fname="testfile";
			createFile(fname,FILESIZE);
			if ((fd=open(fname,O_RDWR))<0){
				fprintf(stderr,"Can’t open file %s for reading and writing",fname);
				perror("");
				return -1;
			}
			if (fstat(fd,&st)==-1){
				fprintf(stderr, "Error calling fstat on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			fsize=(int)st.st_size;
			fprintf(stdout,"The size of the file as reported by fstat is %d\n",fsize);
			fprintf(stdout,"About to MAP_SHARED\n");
			if((map=(char *)mmap(NULL,FILESIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED){
				fprintf(stderr,"Error using mmap with PROT_READ,PROT_WRITE, and MAP_SHARED on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			numbytes=4;
			fprintf(stdout,"About to write %d bytes to offset %d\n",numbytes,FILESIZE);
			for(i=0;i<numbytes;i++){
				map[FILESIZE+i]='a'+i;
			}
			if (fstat(fd,&st)==-1){
				fprintf(stderr, "Error calling fstat on file %s: %s\n",
					fname,strerror(errno));
				return -1;
			}
			newfsize=(int)st.st_size;
			fprintf(stdout,"The new size of the file as reported by fstat is %d\n",newfsize);
			fileDump(fname,fd,FILESIZE,4);
			memDump(map,FILESIZE,4);
			if(fsize==newfsize)
				fprintf(stdout,"In response to question D: No, the file size does not change\n");
			else fprintf(stdout,"In response to question D: Yes, the file size does change\n");
			expandsize=13;
			fprintf(stdout,"About to expand file by %d bytes and write(2) to new end\n",expandsize);
			if(lseek(fd,FILESIZE+expandsize-1,SEEK_SET)==-1){
				fprintf(stderr,"Error using lseek on file %s: %s\n",fname,strerror(errno));
				exit(-1);
			}
			if(write(fd,"a",1)<=0){
				fprintf(stderr,"Error writing to file %s: %s\n",fname,strerror(errno));
			}
			memDump(map,FILESIZE,16);
			buf=fileDump(fname,fd,FILESIZE,16);
			dataremains=1;
			for(i=0;i<numbytes;i++){
				if(map[FILESIZE+i]!=buf[i]){
					dataremains=0;
					break;
				}
			}
			if(dataremains){
				fprintf(stdout,"In response to question E: Yes, the data previously written to the hole remain\n");
			}else{
				fprintf(stdout,"In response to question E: No, the data previously written to the hole do not remain\n");
			}
			free(buf);
			if(close(fd)==-1){
				fprintf(stderr,"Error closing file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(unlink(fname)==-1){
				fprintf(stderr,"Error unlinking file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(munmap(map,FILESIZE)==-1){
				perror("Error calling munmap");
				return -1;
			}
			break;
		case 'F':
			fname="testfile";
			createFile(fname,10);
			if ((fd=open(fname,O_RDWR))<0){
				fprintf(stderr,"Can’t open file %s for reading and writing",fname);
				perror("");
				return -1;
			}
			fprintf(stdout,"About to map a file with MAP_SHARED to a region two pages long \n");
			if((map=(char *)mmap(NULL,8192,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED){
				fprintf(stderr,"Error using mmap with PROT_READ,PROT_WRITE, and MAP_SHARED on file %s: %s\n",
					fname,strerror(errno));
				fprintf(stdout,"In response to question F: No it is not possible to establish a mmap region 2 pages long\n");
				return -1;
			}
			for(i=1;i<32;i++){
				if(i==9 || i==19) continue;
				if(signal(i,sighandler)==SIG_ERR){
					fprintf(stderr,"Error setting signal handler for signal #%d: %s\n",i,strerror(errno));
					return -1;
				}
			}
			for(i=4000;i<8192;i+=4000){
				fprintf(stdout,"About to read from memory in page #%d\n",i/4000);
				if((sn=sigsetjmp(jb,1))!=0){
					fprintf(stdout,"In response to question F: signal #%d was delivered when attempting to access memory from page #%d\n",sn,i/4000);
					continue;
				}
				c=map[i];
				fprintf(stdout,"In response to question F: No signal was delivered when attempting to access memory from page #%d\n",i/4000);
			}
			//Explanation: When a file is smaller than the size of a page the kernel still maps in a whole page and zero fills the rest of 
			//memory. Thus no signal is sent when the process tries to read from a region in the first page even if it is past the end of 
			// the file. However, when the process tries to read from the second page the kernel will be unable to satisfy the page fault 
			//because the backing store does not exist in the file system. Thus the kernel delivers a SIGBUS to the process.
			if(close(fd)==-1){
				fprintf(stderr,"Error closing file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(unlink(fname)==-1){
				fprintf(stderr,"Error unlinking file %s: %s\n",fname,strerror(errno));
				return -1;
			}
			if(munmap(map,10)==-1){
				perror("Error calling munmap");
				return -1;
			}
			break;
	}
	return 0;
}

char * fileDump(char *fname,int fd,int offset,int req){
	if(lseek(fd,offset,SEEK_SET)==-1){
		fprintf(stderr,"Error using lseek on file %s: %s\n",fname,strerror(errno));
		exit(-1);
	}
	char *buf,*tmpbuf;
	if( (buf=(char *)malloc(req))==NULL){
		fprintf(stderr,"Memory allocation failure");
		exit -1;
	}
	tmpbuf=buf;
	int tmpreq=req;
	int n,ret=0;
	while ((n=read(fd,tmpbuf,tmpreq))>0){
		tmpbuf+=n;
		ret+=n;
		tmpreq-=n;
	}
	if (n==-1){
		fprintf(stderr, "Error reading file %s: %s\n",
						fname,strerror(errno));
		exit(-1);
	}
	fprintf(stdout,"File dump starting at offset %d, read req %d, ret %d\n",
		offset,req,ret);
	int i;
	for(i=0;i<ret;i++)
		fprintf(stdout,"<%02X>  ",0xFF&buf[i]);
	fprintf(stdout,"\n<EOF>\n");
	return buf;
}

void memDump(char *map,int offset,int n){
	fprintf(stdout,"Memory dump starting at offset %d for %d bytes\n",offset,n);
	int i;
	for(i=0;i<n;i++)
		fprintf(stdout,"<%02X>  ",0xFF&map[offset+i]);
	fprintf(stdout,"\n");
}
void createFile(char *fname,int fsize){
	fprintf(stdout,"Creating %d byte random file %s\n",
	fsize,fname);
	char str[100];
	sprintf(str,"dd if=/dev/urandom of=%s bs=%d count=1",fname,fsize);
	if(system(str)==-1){
		fprintf(stderr,"Error using the system command to call dd\n");
	}
}
void sighandler(int sn){
	// char *signals[]={"SIGHUP" "SIGINT" "SIGQUIT" "SIGILL" "SIGTRAP" "SIGABRT" "SIGEMT" "SIGFPE" "SIGKILL"
	// 				 "SIGBUS" "SIGSEGV" "SIGSYS" "SIGPIPE" "SIGALARM" "SIGTERM" "SIGUSR1" "SIGUSR2" "SIGCHLD" 
	// 				 "SIGPWR" "SIGWINCH" "SIGURG" "SIGIO" "SIGSTOP" "SIGTSTP" "SIGCONT" "SIGTTIN" "SIGTTOU"
	// 				 "SIGVTALRM" "SIGPROF" "SIGXCPU" "SIGXFSZ"};


	switch(g_argv[1][0]){
		case 'A':
			fprintf(stdout,"In response to question A: when an attempt to write to a mapped region with read only acess is made signal #%d is generated\n",
					sn);
			if(unlink(fname)==-1){
				fprintf(stderr,"Error unlinking file %s: %s\n",fname,strerror(errno));
				exit(-1);
			}
			exit(-1);
			break;
		case 'F':
			siglongjmp(jb,sn);
			break;
	}
}