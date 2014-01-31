// Hillel Weintraub
// 11/12/13
// Operating Systems-Problem 6
// 
// tcp_recv - Accept a connection and copy received data to stdout
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define BUFSIZE 4096

int main(int argc, char const *argv[]){
	if(argc<2){
		fprintf(stderr,"Improper usage\n\tUsage: tcp_recv port\n");
		return -1;
	}
	char const *port=argv[1];
	int s,s2,len;
	struct sockaddr_in sin;
	struct hostent *he;
	if((s=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return -1;
	}
	sin.sin_family=AF_INET;
	sin.sin_port=htons(atoi(port));
	sin.sin_addr.s_addr=INADDR_ANY;
	if (bind(s,(struct sockaddr *)&sin,sizeof sin)<0){
		perror("bind");
		close(s);
		return -1;
	}
	if (listen(s,128)<0){
		perror("listen");
		return -1;
	}

	len=sizeof sin;
	char buf[BUFSIZE];
	char *tmpbuf;
	int n,j,numbytes=0;
	struct timeval stime,currtime;
	if ((s2=accept(s,(struct sockaddr *)&sin,&len))<0){
		perror("accept");
		return -1;
	}
	gettimeofday(&stime,NULL);
	while((n=read(s2,buf,BUFSIZE))>0){
		tmpbuf=buf;
		while((j=write(1,tmpbuf,n))>0 && j<n){
			n-=j;
			tmpbuf+=j;
			numbytes+=j;
		}
		numbytes+=j;
		if(j<=0){
			perror("Error writing to stdout");
			return -1;
		}
	}
	if(n<0){
		perror("Error reading from socket");
		return -1;
	}
	if(close(s2)==-1){
		perror("Error closing socket");
		return -1;
	}
	gettimeofday(&currtime,NULL);
	double starttime=(double)stime.tv_sec+(double)stime.tv_usec/1000000.0;
	double etime=((double)currtime.tv_sec+(double)currtime.tv_usec/1000000.0)-starttime;
	double rate=(numbytes/1048576.0)/etime;
	fprintf(stderr,"A total of %d bytes were sent at a rate of %f MB/s\n",numbytes,rate);
	int portnum=ntohs(sin.sin_port);
	char *ipaddr=inet_ntoa(sin.sin_addr);
	he=gethostbyaddr((char *)&sin.sin_addr,sizeof sin.sin_addr,sin.sin_family);
	char *hostname;
	hostname=(he?he->h_name:"unavailable");
	fprintf(stderr,"Remote endpoint information:\n\tIP adress: %s\n\thost name:%s\n\tport number: %d\n",
		ipaddr,hostname,portnum);

	if(close(s)==-1){
		perror("Error closing socket");
		return -1;
	}
	return 0;
}