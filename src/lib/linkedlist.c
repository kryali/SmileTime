char * list_find(list * head, char * name){
	while(head != NULL){
		if( strcmp(head->name, name) == 0){
			return head->ip;
		}
		head = head->next;
	}
	return "NULL";
}

void list_add(list ** head, char * name, char * ip){
	if(*head == NULL){
		list * newnode = malloc(sizeof(list));
		newnode->name = name;
		newnode->ip = ip;
		newnode->next = NULL;
		*head = newnode;
		return;
	}
	list * curr = *head;

	while(curr->next != NULL){
		// Increment to the end of the list
		curr = curr->next;
	}
	curr->next = malloc(sizeof(list));
	curr = curr->next;
	curr->name = name;
	curr->ip = ip;
	curr->next = NULL;
}

int list_length(list * head){
	int count = 0;
	while( head != NULL){
		count++;
		head = head->next;
	}
	return count;
}

void list_destroy(list ** head){
	if(head == NULL || *head == NULL){
		return;
	}
	list_destroy(&((*head)->next));
	free(*head);
	*head = NULL;
}

void list_print(list * head){
	int count = 0;
	while(head != NULL){
		printf("%d> %s - %s\n", count, head->name, head->ip);
		count++;
		head = head->next;
	}
}
