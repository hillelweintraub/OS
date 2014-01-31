// Hillel Weintraub
// 11/12/13
// Operating Systems-Problem 6
// 
// udp_server - listens for requests on a specified UDP port and returns info to the requester
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
	if(argc<2){
		fprintf(stderr,"Improper usage\n\tUsage: udp_server port\n");
		return -1;
	}
	char const *port=argv[1];
	char buf[BUFSIZE];
	int s,len;
	struct sockaddr_in sin;
	if((s=socket(AF_INET,SOCK_DGRAM,0))<0){
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
	len=sizeof sin;
	FILE *fp;
	while(1){
		if(recvfrom(s,buf,BUFSIZE,0,(struct sockaddr *)&sin,&len)<0){
			perror("recvfrom");
			return -1;
		}
		if(strcmp(buf,"UPTIME")==0){
			if((fp=popen("uptime","r"))==NULL){
				perror("popen");
				continue;
			}
			fgets(buf,BUFSIZE,fp);
			if(sendto(s,buf,strlen(buf)+1,0,(struct sockaddr *)&sin,len)<0){
				perror("sendto");
				continue;
			}
			if(pclose(fp)==-1){
				perror("pclose");
				continue;
			}
		}else if(strcmp(buf,"DATE")==0){
			if((fp=popen("date","r"))==NULL){
				perror("popen");
				continue;
			}
			fgets(buf,BUFSIZE,fp);
			if(sendto(s,buf,strlen(buf)+1,0,(struct sockaddr *)&sin,len)<0){
				perror("sendto");
				continue;
			}
			if(pclose(fp)==-1){
				perror("pclose");
				continue;
			}
		}
	}
	return 0;
}