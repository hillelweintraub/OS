// Hillel Weintraub
// 10/6/13
// Operating Systems-Problem 2
// 
// rls - recursively lists files in filesystem
// USAGE:
//       rls [-u user] [-m mtime] starting_path

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>

char *getRWXString(int i,char * buf){
	buf[0]= (i & 4) ? 'r' : '-';
	buf[1]= (i & 2) ? 'w' : '-';
	buf[2]= (i & 1) ? 'x' : '-';
	return buf;
}

void printPermissions(struct stat st){
	switch (st.st_mode & S_IFMT) {
        case S_IFBLK:  printf("b");        break;
        case S_IFCHR:  printf("c");        break;
        case S_IFDIR:  printf("d");        break;
        case S_IFIFO:  printf("p");        break;
        case S_IFLNK:  printf("l");        break;
        case S_IFREG:  printf("-");        break;
        case S_IFSOCK: printf("s");        break;
    }
    char rwxString[4];
    getRWXString((st.st_mode & S_IRWXU)>>6,rwxString);
    if (st.st_mode & S_ISUID){
    	switch (rwxString[2]){
    		case 'x': rwxString[2]='s';		break;
    		case '-': rwxString[2]='S';		break;
    	}
    }
    printf("%s",rwxString);
    getRWXString((st.st_mode & S_IRWXG)>>3,rwxString);
    if (st.st_mode & S_ISGID){
    	switch (rwxString[2]){
    		case 'x': rwxString[2]='s';		break;
    		case '-': rwxString[2]='S';		break;
    	}
    }
    printf("%s",rwxString);
    getRWXString(st.st_mode & S_IRWXO,rwxString);
    if (st.st_mode & S_ISVTX){
    	switch (rwxString[2]){
    		case 'x': rwxString[2]='t';		break;
    		case '-': rwxString[2]='T';		break;
    	}
    }
    printf("%s ",rwxString);
}

int printfile(char *fname,char const *username,int uid,int mtime){
	struct stat st;
	if (lstat(fname,&st)==-1){
		fprintf(stderr, "Error calling lstat on file %s: %s\n",
			fname,strerror(errno));
		return -1;
	}
	struct passwd *pw=getpwuid(st.st_uid);
	if (username==NULL){
		if (uid!=-1 && st.st_uid!=uid)
			return ((st.st_mode & S_IFMT) == S_IFDIR ) ? 1:0; //return isDir value
	}else if (strcmp(pw->pw_name,username)!=0){
		return ((st.st_mode & S_IFMT) == S_IFDIR ) ? 1:0;
	}
	if (mtime!=0){
		int file_age=(int)time(NULL)-st.st_mtime;
		if (mtime>0){
			if (file_age<mtime)
				return ((st.st_mode & S_IFMT) == S_IFDIR ) ? 1:0;
		}else if(file_age>-mtime){
			return ((st.st_mode & S_IFMT) == S_IFDIR ) ? 1:0;
		}
	}

	printf("%04lx/%lu ",(long)st.st_dev,(long)st.st_ino);
	printPermissions(st);
	printf("%lu ",(long)st.st_nlink);
	if (pw->pw_name==NULL){
		printf("%d\t",(int)st.st_uid);
	}else{
		printf("%s\t",pw->pw_name);
	}
	struct group *gr=getgrgid(st.st_gid);
	if (gr->gr_name==NULL){
		printf("%d\t",(int)st.st_gid);
	}else{
		printf("%s\t",gr->gr_name);
	}
	if ((st.st_mode&S_IFMT)==S_IFCHR || (st.st_mode&S_IFMT)==S_IFBLK){
		printf("%#06x ",(int)st.st_rdev);
	}else{
		printf("%d ",(int)st.st_size);
	}
	char *time=ctime(&st.st_mtime);
	int len=strlen(time);
	time[len-1]='\0'; //GET RID OF NEWLINE
	printf("%s ",time);
	printf("%s",fname);
	if ((st.st_mode&S_IFMT)==S_IFLNK){
		char buf[1024];
		int n=readlink(fname,buf,1023);
		if (n==-1){
			fprintf(stderr, "\nError reading symbolic link %s: %s\n",
			fname,strerror(errno));
		}
		else{
			buf[n]='\0';
		}
		printf(" -> %s\n",buf);
	}
	else{
		printf("\n");
	}
	return ((st.st_mode & S_IFMT) == S_IFDIR ) ? 1:0;  
}

void readDir(char *fn,char const *username,int uid,int mtime){
	DIR *dirp;
	struct dirent *de;
	int isDir=printfile(fn,username,uid,mtime);
	if(!isDir) 
		return;
	if (!(dirp=opendir(fn))){ 
		fprintf(stderr, "Can not open directory %s: %s\n",
			fn,strerror(errno));
		return;
	}
	char fname[1024]; 
	int bufsize=1024, len;
	errno=0;
	while (de=readdir(dirp)){
		len=strlen(fn)+strlen(de->d_name)+2;
		if(len>bufsize){
			fprintf(stderr, "Can not read directory entry from %s Path_name is too long.\n",
			fn);
			continue;
		}
		strcpy(fname,fn);
		strcat(fname,"/");
		strcat(fname,de->d_name);
		if (strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0){
			continue;
		}
		if (de->d_type==4){ //file is a directory
			readDir(fname,username,uid,mtime);
		}
		else{
			printfile(fname,username,uid,mtime);
		}
	}
	if (errno){
		fprintf(stderr, "Error reading directory %s: %s\n",
			fn,strerror(errno));
	}
	closedir(dirp);
}

int isNumber(char *s){
	int isNumber=1;
	int len=strlen(s);
	int i;
	for(i=0;i<len;i++){
		if(!isdigit(s[i])){
			isNumber=0;
			break;
		}
	}
	return isNumber; 
}

void parseArgs(int argc,char const *argv[],char const **username,int *uid,int *mtime,char **starting_path){
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	static char usage[] = "usage: %s [-u user] [-m mtime] starting_path\n";

	while ((c = getopt(argc, argv, "u:m:")) != -1){
		switch (c) {
		case 'u':
			if (isNumber(optarg))
			 *uid=atoi(optarg);
			else 
				*username=optarg;
			break;
		case 'm':
			*mtime=atoi(optarg);
			break;
		case '?':
			err = 1;
			break;
		}
	}
	if ((optind+1) > argc) {	// need at least one argument
		fprintf(stderr, "%s: missing starting_path\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(-1);
	} else if (err) {
		fprintf(stderr, usage, argv[0]);
		exit(-1);
	}
	*starting_path=argv[optind];
}

int main(int argc, char const *argv[])
{
	char const *username=NULL;
	int uid=-1;
	int mtime=0;
	char *starting_path;
	parseArgs(argc,argv,&username,&uid,&mtime,&starting_path);
	readDir(starting_path,username,uid,mtime);
	return 0;
}