#include "include.h"

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


