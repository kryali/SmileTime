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

int xwrite(int fd, HTTP_packet* np){
	int ret = 0;
	//ret = write(fd, np->message, np->length);
  ret = send(fd, np->message, np->length, NULL);
	if(ret == -1){
		perror("write");
		exit(1);
	}
	return ret;
}

int xread(int fd, HTTP_packet* np){
	int ret = 0;
	//ret = read(fd, np->message, np->length);
	ret = recv(fd, np->message, np->length, NULL);
	if(ret == -1){
		perror("write");
		exit(1);
	}
	return ret;
}
