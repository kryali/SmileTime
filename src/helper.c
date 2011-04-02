#include "helper.h"

char* strstp(char * str, char * stp, int * size){
	char * loc = strstr(str, stp);
	*size = loc-str+1;
	char * retstr = malloc(*size);
	memset(retstr, 0, *size);
	strncpy(retstr, str, *size-1);
	retstr[*size-1] = '\0';
	return retstr;
}

int xwrite(int fd, void * buf, int len){
	int ret = 0;
	ret = write(fd, buf, len);
	if(ret == -1){
		perror("write");
		exit(1);
	}
	return ret;
}

int xread(int fd, void * buf, int len){
	int ret = 0;
	ret = read(fd, buf, len);
	if(ret == -1){
		perror("write");
		exit(1);
	}
	return ret;
}
