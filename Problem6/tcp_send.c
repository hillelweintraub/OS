// Hillel Weintraub
// 11/12/13
// Operating Systems-Problem 6
// 
// tcp_send - copy a stream of bytes from standard input to a TCP port
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define BUFSIZE 4096

int main(int argc, char const *argv[]){
	if(argc<3){
		fprintf(stderr,"Improper usage\n\tUsage: tcp_send hostname port\n");
		return -1;
	}
	char const *hostname=argv[1];
	char const *port=argv[2];
	int s;
	struct sockaddr_in sin;
	struct hostent *he;
	if((s=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return -1;
	}
	struct linger so_linger;
	so_linger.l_onoff=1;
	so_linger.l_linger=30;
	if(setsockopt(s,SOL_SOCKET,SO_LINGER,&so_linger,sizeof so_linger)==-1){
		perror("setsockopt");
		return -1;
	}
	sin.sin_family=AF_INET;
	sin.sin_port=htons(atoi(port));
	if((sin.sin_addr.s_addr=inet_addr(hostname))==-1){
		if(!(he=gethostbyname(hostname))){
			fprintf(stderr,"Unknown host: %s",hostname);
			herror(" ");
			return -1;
		}
		memcpy(&sin.sin_addr.s_addr,he->h_addr_list[0],sizeof sin.sin_addr.s_addr);
	}

	char buf[BUFSIZE];
	char *tmpbuf;
	int n,j,numbytes=0;
	struct timeval stime,currtime;
	if(connect(s,(struct sockaddr *)&sin,sizeof sin)==ECONNREFUSED){
		fprintf(stderr,"Error connecting to host %s on port %s: %s\n",hostname,port,strerror(errno));
		close(s);
		return -1;
	}
	gettimeofday(&stime,NULL);
	while((n=read(0,buf,BUFSIZE))>0){
		tmpbuf=buf;
		while((j=write(s,tmpbuf,n))>0 && j<n){
			n-=j;
			tmpbuf+=j;
			numbytes+=j;
		}
		numbytes+=j;
		if(j<=0){
			fprintf(stderr,"Error writing to host %s on port %s: %s\n",hostname,port,strerror(errno));
			return -1;
		}
	}
	if(n<0){
		perror("Error reading from stdin");
		return -1;
	}
	if(close(s)==-1){
		fprintf(stderr,"Error closing socket connected to host %s on port %s: %s\n",hostname,port,strerror(errno));
		return -1;
	}
	gettimeofday(&currtime,NULL);
	double starttime=(double)stime.tv_sec+(double)stime.tv_usec/1000000.0;
	double etime=((double)currtime.tv_sec+(double)currtime.tv_usec/1000000.0)-starttime;
	double rate=(numbytes/1048576.0)/etime;
	fprintf(stderr,"A total of %d bytes were sent at a rate of %f MB/s\n",numbytes,rate);
	return 0;
}