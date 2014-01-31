#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]){
	char buf[15];
	strcpy(buf,"Hello World\n");
	write(1,buf,strlen(buf));
	return 0;
}