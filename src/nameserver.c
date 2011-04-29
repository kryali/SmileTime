#include "nameserver.h"

int main(){
	iplist = NULL;
//	populate();
	/*
	list_print(iplist);
	*/
	init_server();
	message_listen();
	return 0;
}

void populate(){
	list_add(&iplist, "Kiran", "127.0.0.1", "1337", TCP);
	list_add(&iplist, "John", "127.0.0.2","1337", TCP);
	list_add(&iplist, "Cliff", "127.0.0.3", "1337", TCP);
	list_add(&iplist, "Batman", "127.0.0.4", "1337", UDP);
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
	unsigned int addr_size = sizeof(struct sockaddr_storage);
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
	char protocol = *msg;
	printf("PROTOCOL %c\n", protocol);

	list_add(&iplist, name, ip, port,protocol);
  server_list_prune(iplist);
	list_print(iplist);
}



char * nameServerMsg(char * name, char * ip, char * port, char protocol, int * size){
    *size = strlen(ip) + strlen(port) + 5 + strlen(name);
    char * msg = malloc(*size);
    memset(msg, 0, *size);
    strcat(msg, name);
    strcat(msg, "#");
    strcat(msg, ip);
    strcat(msg, ":");
    strcat(msg, port);
    strcat(msg, "#");
    strcat(msg, &protocol);
    msg[*size-1] = '\0';
    return msg;
}

void server_list_prune(list *head) {
  int i;
  list *removed;
  list *new_head;
	while(head != NULL){
    for( i=0; i < strlen(head->name); i++ ) 
    {
      if( head->name[i] < 48 || head->name[i] > 122 )
      {
        new_head = head->next;
        removed = list_remove(iplist, head);
        if( removed == iplist )
          iplist = new_head;
        break;
      }
    }
		head = head->next;
	}
}

char * server_find(char * name){
	int size = 0;
	list * f = list_find(iplist, name);
	if( strcmp(f->name, "NULL") == 0){
		return "(null)";
	}
	return nameServerMsg(f->name, f->ip, f->port,f->protocol, &size);
}

void handle_connection(int fd){
	printf("[NAMESERVER] reading header code..\n");
	char headerCode = 10; //Hardcode some value that isn't any of the defined properties
	if( read (fd, &headerCode, 1) != 1){
		perror("read");
		exit(1);
	}
	int size = 0;
	char * msg = NULL;

  server_list_prune(iplist);

	switch(headerCode){
		case FIND:
			printf("[NAMESERVER] FIND received!\n");
			
			// Receive size of message
			if( read( fd, &size, sizeof(int)) == -1){
				perror("read");
				exit(1);
			}
			printf("Size received %d\n", size);
			size += 1;	
			msg = malloc(size);
			// Read the name
			memset(msg, 0, size);
			read(fd, msg, size-1);
	
			printf("Message read, %s\n", msg);

			// return the found name
			char * ip = server_find(msg);
			printf("Found: %s for %s\n", ip, msg);
			size = strlen(ip);
			write(fd, &size, sizeof(int));
			printf("Size: %d\n", size);
			write(fd, ip, size);
			printf("Message sent\n");
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
      	case LIST:
			printf("[NAMESERVER] LIST received!\n");
			list_print(iplist);
			list * temp = iplist;			
			int size = 0;
			int count = 0;
			while(temp != NULL){
				printf("%s\n", temp->name);
				size += strlen(temp->name);
				temp = temp->next;
				count++;
			}
			size += count + 1;
			char * retStr = malloc(size);
			retStr[0] = '\0';
			memset(retStr, 0, size);

			temp = iplist;			
			int i = 0;
			while(temp != NULL){
				strcat(retStr, temp->name);
				if( i != count-1)
					strcat(retStr, "#");
				printf("Count: %d/%d\n", i,count);
				temp = temp->next;
				i++;
			}
			char retSize = size;	
			write(fd, &retSize, 1); 

			printf("Sending string: %s\n", retStr);
			if (write( fd, retStr, strlen(retStr)+1) == -1){
				perror("write");	
			}
//			free(retStr);
			break;
		default:
			printf("[NAMESERVER] Invalid header code\n");
			break;
	}
}
