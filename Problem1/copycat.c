// Hillel Weintraub
// 9/15/13
// Operating Systems-Problem 1
// 
// copycat - concatenate and copy files
// USAGE:
//       copycat [-b ###] [-o outfile] infile1 [...infile2...]
//       copycat [-b ###] [-o outfile]

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_BSIZE 4096

void handleCallingError();

void parseArgs(int *bufsize,int *ofile_idx,int *ifile_idx,
				int argc,char const *argv[] );
		
int openOutFile(int ofile_idx, char const *argv[]);

int openInFile(int ifile_idx, char const *argv[]);

void writeFile(char *buf,int numbytes,int ofile_fd, int ofile_idx, char const *argv[]);

void copyFile(int ifile_fd,int ifile_idx,int ofile_fd,int ofile_idx,
				 char *buf,int bufsize,char const *argv[]);
void closeFile(int ofile_fd,int ofile_idx, char const *argv[]);

int main(int argc, char const *argv[]){
	int bufsize=DEFAULT_BSIZE;
	int ofile_idx=-1;         //-1 signifies no outfile provided
	int ifile_idx=1;             //-1 signifies no infile provided
	parseArgs(&bufsize,&ofile_idx,&ifile_idx,argc,argv);
	char *buf= (char*) malloc(bufsize);
	if(buf==NULL){
		fprintf(stderr,"Unable to allocate memory to the buffer");
		exit(-1);
	} 

	int ofile_fd=openOutFile(ofile_idx,argv);
	
	if (ifile_idx<0){	//no input files provided
		copyFile(0,ifile_idx,ofile_fd,ofile_idx, buf,bufsize,argv);
	}
	else{				//loop through input files
		int ifile_fd;
		for (; ifile_idx < argc; ++ifile_idx){
			ifile_fd=openInFile(ifile_idx,argv);
			copyFile(ifile_fd,ifile_idx,ofile_fd,ofile_idx, buf,bufsize,argv);
			closeFile(ifile_fd,ifile_idx,argv);
		}
	}
	closeFile(ofile_fd,ofile_idx,argv);
	free(buf);
	return 0;
}

//prints an error message if program was called incorrectly
void handleCallingError(){
	char const *usage_string1="copycat [-b ###] [-o outfile] infile1 [...infile2...]";
	char const *usage_string2="copycat [-b ###] [-o outfile]";
	fprintf(stderr,"Improper usage!\nUSAGE:\n\t%s\n\t%s\n",usage_string1,usage_string2);
	exit(-1);
}

void parseArgs(int *bufsize,int *ofile_idx,int *ifile_idx,
	int argc,char const *argv[] ){
	if (argc>1 && strcmp("-b",argv[1])==0){
		if (argc>2){
			if((*bufsize=atoi(argv[2]))<=0){
				fprintf(stderr,"Error: Buffer size %d is invalid. Buffer size must be a positive integer\n",
					*bufsize);
				exit(-1);
			}
			*ifile_idx=3;
		}
		else{
			handleCallingError();
		}
		if (argc>3 && strcmp("-o",argv[3])==0){
			if (argc>4){
				*ofile_idx=4;
				*ifile_idx=5;
			}	
			else{
				handleCallingError();
			}
		}
	}
	else{	
		if (argc>1 && strcmp("-o",argv[1])==0){
		 	if (argc>2){
		 		*ofile_idx=2;
		 		*ifile_idx=3;
		 	}
		 	else{
		 		handleCallingError();
		 	}
		 	if (argc>3 && strcmp("-b",argv[3])==0){
		 		if (argc>4){
		 			if((*bufsize=atoi(argv[4]))<=0){
						fprintf(stderr,"Error: Buffer size %d is invalid. Buffer size must be a positive integer\n",
							    *bufsize);
						exit(-1);
					}
		 			*ifile_idx=5;
		 		}
		 		else{
		 			handleCallingError();
		 		}
		 	}
		}
	}
	if (argc<=*ifile_idx){
		*ifile_idx=-1;
	}
		
}

int openOutFile(int ofile_idx, char const *argv[]){
	int ofile_fd=ofile_idx<0?1:open(argv[ofile_idx],O_WRONLY|O_CREAT|O_TRUNC,0777);
	if (ofile_fd==-1){
		extern int errno;
		fprintf(stderr, "Can't open file %s for writing: %s\n",
			    argv[ofile_idx],strerror(errno));
		exit(-1);
	}
	return ofile_fd;
}

int openInFile(int ifile_idx, char const *argv[]){
	int ifile_fd;
	if (( ifile_fd= (strcmp(argv[ifile_idx],"-")==0?0:open(argv[ifile_idx],O_RDONLY)) ) <0){
		extern int errno;
		fprintf(stderr, "Can't open file %s for reading: %s\n",
				argv[ifile_idx],strerror(errno));
		exit(-1);
	}
	return ifile_fd;
 }

void writeFile(char *buf,int numbytes,int ofile_fd, int ofile_idx, char const *argv[]){
	int j;
	while ((j=write(ofile_fd,buf,numbytes))>0 && j<numbytes){
		buf+=j;
		numbytes-=j;
	}
	if (j<=0){
		extern int errno;
		if (ofile_fd==1){
			fprintf(stderr, "Error writing to standard output: %s\n",
				strerror(errno));
		}
		else{
			fprintf(stderr, "Error writing to file %s: %s\n",
				argv[ofile_idx],strerror(errno));
		}
		exit(-1);
	}
}

//reads from the input file and writes contents to the output file
void copyFile(int ifile_fd,int ifile_idx,int ofile_fd,int ofile_idx, char *buf,int bufsize,char const *argv[]){
	int n;
	while ((n=read(ifile_fd,buf,bufsize))>0){
				writeFile(buf,n,ofile_fd,ofile_idx, argv);
			}
			if (n==-1){
				extern int errno;
				if (ifile_fd==0){
					fprintf(stderr, "Error reading from standard input: %s\n",
						strerror(errno));
				}
				else{
					fprintf(stderr, "Error reading file %s: %s\n",
						argv[ifile_idx],strerror(errno));
				}
				exit(-1);
			}
}

void closeFile(int ofile_fd,int ofile_idx, char const *argv[]){
	if (ofile_fd>2){
		int retval=close(ofile_fd);
		if (retval==-1){
			extern int errno;
			fprintf(stderr, "Error closing file %s: %s\n",
				argv[ofile_idx],strerror(errno));
			exit(-1);
		}
	}
}