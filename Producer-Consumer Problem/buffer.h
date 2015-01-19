typedef int buffer_item;
#define BUFFER_SIZE 5
buffer_item buffer[BUFFER_SIZE]; //Circular Queue
int head = 0;
int tail = 0;
int counter = 0;

//insert item into buffer
int insert_item(buffer_item item)
{
	if (counter == BUFFER_SIZE)
		return -1; //buffer is full

	buffer[tail] = item;
	tail = (tail + 1) % BUFFER_SIZE;
	++counter;
	
	return 0;

	//tail = (tail + 1) % BUFFER_SIZE;
	//if (tail == head)
	//	return -1; //buffer is full

	//buffer[tail] = item;
	//return 0;
}

//remove an item from the buffer
int remove_item(buffer_item* item)
{
	if (counter == 0)
		return -1;

	*item = buffer[head];
	head = (head + 1) % BUFFER_SIZE;
	--counter;
	return 0;
}
