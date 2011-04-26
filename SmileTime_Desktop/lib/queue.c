queue_push( struct queue *queue, void* data )
{
  // Create the new node for the data
  newNode = malloc(sizeof(queue_node));
  newNode->data = data;

  // Create queue if it does not exist
  if( queue == NULL )
  {
    queue = malloc(sizeof(struct queue));
    queue->head = newNode;
    queue->tail = newNode;
  }

  newNode->next = queue->tail;
  queue->tail = newNode;
}

queue_pop( struct queue *queue )
{
  queue_node* this;
  void* ret;

  if( queue == NULL )
  {
    printf( "queue is NULL\n" );
    return -1;
  }
  if( queue->head == NULL )
  {
    printf( "queue head is NULL\n" );
    return -1;
  }

  // Save the element that we want to return
  ret = queue->head->data;

  // Free memory of current head
  free(queue->head);

  // This was the only element in the queue
  if( queue->head == queue->tail )
    queue->head = queue->tail = NULL;
  else
  {
    this = queue->tail;

    // Reset head
    while( this != queue->head )
    {
      if( this->next == queue->head )
      {
        queue->head = this->next;
        break;
      }

      this = this->next;
    }
  }

  return ret;
}

list * list_find(list * head, char * name){
	printf("%d\n", head);
	while(head != NULL){
		if( strcmp(head->name, name) == 0){
			return head;
		}
		head = head->next;
	}
	return "NULL";
}

void list_add(list ** head, char * name, char * ip, char * port, char protocol){
	if(*head == NULL){
		list * newnode = malloc(sizeof(list));
		newnode->name = name;
		newnode->ip = ip;
		newnode->port = port;
		newnode->protocol = protocol;
		newnode->next = NULL;
		*head = newnode;
		return;
	}
	list * curr = *head;

	while(curr->next != NULL){
		// Increment to the end of the list
		if( strcmp(curr->name, name) == 0)
			return; // Already exists
		curr = curr->next;
	}
	curr->next = malloc(sizeof(list));
	curr = curr->next;
	curr->name = name;
	curr->port = port;
	curr->ip = ip;
	curr->protocol = protocol;
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
		printf("%d> %s - %s:%s [%c]\n", count, head->name, head->ip, head->port,head->protocol);
		count++;
		head = head->next;
	}
}
