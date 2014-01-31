
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char const *argv[]){
	FILE *fp;
	if (argc>1){
           if ( (fp = fopen(argv[1],"r") ) == NULL){
           		fprintf(stderr,"Error opening file %s: %s\n",argv[1],strerror(errno));
           		return -1;
           }
	}else fp=stdin;
	int bufsize=100;
	char **av;
	if( (av= (char **)malloc( bufsize*sizeof (char *) )) ==NULL){
		fprintf(stderr,"Memory allocation failure");
		return -1;
	}
	char *rov[3]; //redirection_operation vector
	int i;
	for(i=0;i<3;i++)
		rov[i]=NULL;
	char *token;
	char *line=NULL;
	int fd,flag,status,pid,len;
   	struct rusage ru;
	struct timeval tv;
	while (getline(&line,&len, fp) != -1) {
		if (line[0]=='#') continue;
		//parse line and get command and args
		av[0]=strtok(line," \t\n");
		for(i=1; (token = strtok(NULL," \t\n")) != NULL;i++ ){
			if(token[0]=='<' || token[0]=='>' || (token[0]=='2' && token[1]=='>')){
				rov[0]=token; 
				break;
			}
			if (i>bufsize-2){
				bufsize*=2;
				if((av=(char **)realloc(av,bufsize))==NULL){
					fprintf(stderr,"Memory allocation failure");
					return -1;
				}
			}
			av[i]=token;
		} 
		av[i]=NULL;
		for( i=1; token != NULL && i<3;i++,token = strtok(NULL," \t\n") ){
			rov[i]=token;
		}
		switch(pid=fork()){
			case -1:
				perror("fork failed"); exit(-1);
				break; 
			case 0:
				//in child
				//I/O redirect
				for(i=0;i<3 && rov[i]!=NULL;i++){
					switch(rov[i][0]){
						case '<':
							token=strtok(rov[i],"<");
							if ((fd=open(token,O_RDONLY))<0){
								fprintf(stderr,"Can’t open file %s for reading",token);
								perror("");
								return -1;
							}
							if (dup2(fd,0)<0) {
								fprintf(stderr, "Can't dup2 file %s to stdin: %s\n",token,strerror(errno));
								return -1;
							}
							if(close(fd)==-1 ){
								fprintf(stderr, "Error closing file %s: %s\n",
								token,strerror(errno));
								return -1;
							}
							break;
						case '>':
							flag= rov[i][1]=='>'?O_CREAT|O_APPEND|O_WRONLY:O_CREAT|O_TRUNC|O_WRONLY;
							token=strtok(rov[i],">");
							if ((fd=open(token,flag,0666))<0){
								fprintf(stderr,"Can’t open file %s for writing",token);
								perror("");
								return -1;
							}
							if (dup2(fd,1)<0) {
								fprintf(stderr,"Can’t dup2 file %s to stdout: %s\n",token,strerror(errno));
								return -1;
							}
							if(close(fd)==-1 ){
								fprintf(stderr, "Error closing file %s: %s\n",
								token,strerror(errno));
								return -1;
							}
							break;
						case '2':
							flag= rov[i][2]=='>'?O_CREAT|O_APPEND|O_WRONLY:O_CREAT|O_TRUNC|O_WRONLY;
							token=strtok(rov[i],"2>");
							if ((fd=open(token,flag,0666))<0){
								fprintf(stderr,"Can’t open file %s for writing",token);
								perror("");
								return -1;
							}
							if (dup2(fd,2)<0) {
								fprintf(stderr,"Can’t dup2 file %s to stderr: %s\n",token,strerror(errno));
								return -1;
							}
							if(close(fd)==-1 ){
								fprintf(stderr, "Error closing file %s: %s\n",
								token,strerror(errno));
								return -1;
							}
							break;
					}
				}
				if (fp!=stdin){
					if(fclose(fp)==EOF){
						fprintf(stderr, "Error closing file %s: %s\n",argv[1],strerror(errno) );
						return -1;
					}
				}
				fprintf(stdout, "Executing command %s with arguments ",av[0] );
				for(i=1;av[i]!=NULL;i++){
					fprintf(stdout, "\"%s\" ",av[i]);
				}
				fprintf(stdout, "\n");
				if (execvp(av[0],av)==-1){
					fprintf(stderr,"Error calling exec: %s\n",strerror(errno));
				}
				break;
			default:
				//in parent
				if(gettimeofday(&tv,NULL)==-1){
					fprintf(stdout, "gettime of day failed: %s,\n",strerror(errno));
					return -1;
				}
				double seconds=tv.tv_sec+tv.tv_usec/1000000.0;
				if(wait3(&status,0,&ru)==-1){
					perror("wait3 failed");
					return -1;
				}else{
					if(gettimeofday(&tv,NULL)==-1){
						fprintf(stdout, "gettime of day failed: %s,\n",strerror(errno));
						return -1;
					}
					seconds=tv.tv_sec+tv.tv_usec/1000000.0 - seconds;
					if(WIFEXITED(status)){
						fprintf(stdout, "Command returned with return code %d,\n",WEXITSTATUS(status));

					}else if(WIFSIGNALED(status)){
						fprintf(stdout, "Command was terminated by signal %d,\n",WTERMSIG(status));
					}
					fprintf(stdout,"consuming %f real seconds, %ld.%.6d user, %ld.%.6d system\n",seconds,
						ru.ru_utime.tv_sec,ru.ru_utime.tv_usec,
						ru.ru_stime.tv_sec,ru.ru_stime.tv_usec);

				}
				break;
		}

    }
    fprintf(stdout,"end of file\n");
    if (fp!=stdin){
					if(fclose(fp)==EOF){
						fprintf(stderr, "Error closing file %s: %s\n",argv[1],strerror(errno) );
						return -1;
					}
				}
    free(line);
    free(av);
	return 0;
}