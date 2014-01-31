// Hillel Weintraub
// 11/12/13
// Operating Systems-Problem 6
// 
// udp_client - Sends a request to the specified UDP server, receives the response, and prints it out
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
	if(argc<4){
		fprintf(stderr,"Improper usage\n\tUsage: udp_client hostname port request_string\n");
		return -1;
	}
	char const *hostname=argv[1];
	char const *port=argv[2];
	char const *request_string=argv[3];
	char buf[BUFSIZE];
	int s,len;
	struct sockaddr_in sin;
	struct hostent *he;
	if((s=socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("socket");
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
	len=sizeof sin;
	if(sendto(s,request_string,strlen(request_string)+1,0,(struct sockaddr *)&sin,len)<0){
		perror("sendto");
		return -1;
	}
	if(recvfrom(s,buf,BUFSIZE,0,(struct sockaddr *)&sin,&len)<0){
		perror("recvfrom");
		return -1;
	}
	fprintf(stdout,"%s\n",buf);
	if(close(s)==-1){
		perror("Error closing socket");
		return -1;
	}
	return 0;
}