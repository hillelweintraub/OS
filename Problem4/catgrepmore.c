// Hillel Weintraub
// 10/20/13
// Operating Systems-Problem 4
// 
// catgrepmore - run files through a grep more pipeline
// USAGE:
//       catgrepmore pattern infile1 [...infile2...]

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#define BUFSIZE 4096
int i,numbytes;
void int_handler(int sn){
	fprintf(stderr, "SIGINT received:\n\tTotal files fully processed:%d\n\tTotal bytes processed:%d\n",
			i-2,numbytes);
	exit(-1);
}

int main(int argc, char const *argv[]){
	
	if(argc<3){
		char const *usage_string="catgrepmore pattern infile1 [...infile2...]";
		fprintf(stderr, "Improper usage\nUsage:\n\t%s\n",usage_string );
		return -1;
	}
	
	char const *pattern=argv[1];
	char *av[3];
	int ifile_fd;
	char buf[BUFSIZE];
	if(signal(SIGINT,int_handler)==SIG_ERR){
		perror("Error setting handler for SIGINT");
		return -1;
	}
	if(signal(SIGPIPE,SIG_IGN)==SIG_ERR){
		perror("Error setting disposition of SIGPIPE signal to SIG_IGN");
		return -1;
	}
	
	int cgm2grep[2],grep2more[2];
	for (i=2;i<argc;i++){
		//set up pipline
		if(pipe(cgm2grep)<0 || pipe(grep2more)<0){
			perror("can't create pipe");
			return -1;
		}

		if ((ifile_fd=open(argv[i],O_RDONLY)) <0){
			fprintf(stderr, "Can't open file %s for reading: %s\n",
					argv[i],strerror(errno));
			exit(-1);
		}
		switch(fork()){
			case -1:
				perror("fork failed"); return -1;
				break;
			case 0:   //in the child: grep
				if(dup2(cgm2grep[0],0)<0){
					perror("Can't dup2 pipe to grep's stdin");
					return -1;
				} 
				if(dup2(grep2more[1],1)<0){
					perror("Can't dup2 pipe to grep's stdout");
					return -1;
				}
				if(close(cgm2grep[0])==-1 || close(cgm2grep[1])==-1 || close(grep2more[0])==-1 || close(grep2more[1])==-1){
					perror("Error closing pipe");
					return -1;
				}
				if(close(ifile_fd)==-1){
					fprintf(stderr,"Error closing file %s\n",argv[i]);
					return -1;
				}
				if(signal(SIGPIPE,SIG_DFL)==SIG_ERR){
					perror("Error setting disposition of SIGPIPE signal to SIG_DFL");
					return -1;
				}
				av[0]="grep";av[1]=pattern;av[2]=NULL;
				if (execvp(av[0],av)==-1){
					perror("Error calling exec");
					return -1;
				}
				break;
			default:  //in the parent 
				switch(fork()){
					case -1:
						perror("fork failed"); return -1;
						break;
					case 0:   //in the child: more
						if(dup2(grep2more[0],0)<0){
							perror("Can't dup2 pipe to more's stdin");
							return -1;
						}
						if(close(cgm2grep[0])==-1 || close(cgm2grep[1])==-1 || close(grep2more[0])==-1 || close(grep2more[1])==-1){
							perror("Error closing pipe");
							return -1;
						}
						if(close(ifile_fd)==-1){
							fprintf(stderr,"Error closing file %s\n",argv[i]);
							return -1;
						}
						av[0]="more";av[1]=NULL;
						if (execvp(av[0],av)==-1){
							perror("Error calling exec");
							return -1;
						}
						break;
					default:  //in the parent 
						if(close(cgm2grep[0])==-1 || close(grep2more[0])==-1 || close(grep2more[1])==-1){
							perror("Error closing pipe");
							return -1;
						}
						//read from input file and write to pipeline
						int n;
						while ((n=read(ifile_fd,buf,BUFSIZE))>0){

							int j;
							char *writebuf=buf;
							while ((j=write(cgm2grep[1],writebuf,n))>0 && j<n){
								numbytes+=j;
								writebuf+=j;
								n-=j;
							}
							if (j<=0){
								if(errno==EPIPE){//handle the user quiting more
									break; //go to next input file
								}else{
									perror("Error writing to pipline");
									return -1;
								}
							}
							numbytes+=j;
						}
						if (n==-1){
							fprintf(stderr,"Error reading from file %s\n",argv[i]);
							return -1;
						}
						//finished piping contents of input file
						if(close(cgm2grep[1])==-1){
							perror("Error closing pipe");
							return -1;
						}
						if(close(ifile_fd)==-1){
							fprintf(stderr,"Error closing file %s\n",argv[i]);
							return -1;
						}
						int status;
						if(wait(&status)==-1){
							perror("Wait failed");
							return -1;
						}
						if(wait(&status)==-1){
							perror("Wait failed");
							return -1;
						}
						break;
				}
				break;
		}
	}
	return 0;
}
