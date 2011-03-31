#include "nameserver.h"

int main(){
	iplist = NULL;
	list_add(&iplist, "Kiran", "127.0.0.1", "1337", TCP);
	list_add(&iplist, "John", "127.0.0.2","1337", TCP);
	list_add(&iplist, "Cliff", "127.0.0.3", "1337", TCP);
	list_add(&iplist, "Batman", "127.0.0.4", "1337", UDP);
	list_print(iplist);


	init_server();
	message_listen();
	return 0;
}


void init_server(){

	// Open up a socket
	nameserver_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if( nameserver_socket == -1 ){
		perror("socket");
		exit(1);
	}

	// Build sock addr
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port =  htons(NAMESERVER_LISTEN_PORT);

	int addr_size = sizeof(struct sockaddr_in);

	int optval = 1;
    if( (setsockopt(nameserver_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) == -1){
        perror("setsockopt");
        exit(1);
    }

	// Bind socket
	if( bind( nameserver_socket, &addr, addr_size ) == -1) {
		perror("bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( nameserver_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}

	printf("[NAMESERVER] Server initialization complete.\n");

}

void message_listen(){
	struct sockaddr_storage their_addr;
	int addr_size = sizeof(struct sockaddr_storage);
	int acceptfd;
	while(1){
		memset(&their_addr, 0, sizeof(struct sockaddr_storage));

		printf("Waiting for a connection...\n");
		if(( acceptfd = accept( nameserver_socket, (struct sockaddr *)&their_addr, &addr_size )) == -1 ){
			perror("accept");
			exit(1);
		}
		printf("Connection recieved!\n");
		handle_connection(acceptfd);
	}
}

void add_server(char * msg){

	int size = 0;
	char * name = strstp(msg, "#", &size);
	printf("Name %s\n", name);
	msg += size;
	char * ip = strstp(msg, ":", &size);
	printf("IP %s\n", ip);
	msg += size;
	char * port = strstp(msg, "#", &size);
	printf("PORT %s\n", port);
	msg += size;
	char * prot = msg;
	printf("PROT %s\n", msg);

	list_add(&iplist, name, ip, port,msg);
	list_print(iplist);
}

char* strstp(char * str, char * stp, int * size){
	char * loc = strstr(str, stp);
	*size = loc-str+1;
	char * retstr = malloc(*size);
	memset(retstr, 0, *size);
	strncpy(retstr, str, *size-1);
	retstr[*size-1] = '\0';
	return retstr;
}


char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size){
    *size = strlen(ip) + strlen(port) + 5 + strlen(name);
    char * msg = malloc(*size);
    memset(msg, 0, *size);
    strcat(msg, name);
    strcat(msg, "#");
    strcat(msg, ip);
    strcat(msg, ":");
    strcat(msg, port);
    strcat(msg, "#");
    strcat(msg, protocol);
    msg[*size-1] = '\0';
    return msg;
}

char * server_find(char * name){
	int size = 0;
	list * f = list_find(iplist, name);
	return nameServerMsg(f->name, f->ip, f->port,f->protocol, &size);
}

void handle_connection(int fd){
	char headerCode = 10; //Hardcode some value that isn't any of the defined properties
	if( read (fd, &headerCode, 1) != 1){
		perror("read");
		exit(1);
	}
	int size = 0;
	char * msg = NULL;
	switch(headerCode){
		case FIND:
			printf("[NAMESERVER] FIND received!\n");
			
			// Receive size of message
			if( read( fd, &size, sizeof(int)) == -1){
				perror("read");
				exit(1);
			}
			
			msg = malloc(size);
			// Read the name
			memset(msg, 0, size);
			read(fd, msg, size);

			// return the found name
			char * ip = server_find(msg);
			printf("Found: %s for %s\n", ip, msg);
			size = strlen(ip);
			write(fd, &size, sizeof(int));
			printf("Size: %d\n", size);
			write(fd, ip, size);
			break;
		case ADD:
			printf("[NAMESERVER] ADD received!\n");
			
			// Receive size of message
			if( read( fd, &size, sizeof(int)) == -1){
				perror("read");
				exit(1);
			}
			printf("Incoming message size: %d bytes\n", size);
			msg = malloc(size);
			memset(msg, 0, size);

			if( read( fd, msg, size) == -1){
				perror("read");
				exit(1);
			}
			printf("[NAMESERVER] received msg: %s\n", msg);
			add_server(msg);
			free(msg);
			break;
		case EXIT:
			printf("[NAMESERVER] EXIT received!\n");
			break;
		default:
			printf("[NAMESERVER] Invalid header code\n");
			break;
	}
}
