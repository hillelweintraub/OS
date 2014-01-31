#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	double d=2/3.0e1; 
	fprintf(stderr,"%f\n",d );
	return 0;
}