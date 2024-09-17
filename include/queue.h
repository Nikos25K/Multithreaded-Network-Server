#ifndef QUEUE_H
#define QUEUE_H

// A queue data structure
typedef struct queue *Queue;

//freeing function pointer
typedef void (*FreePtr)(void* data);

//compare function pointer used for the queue
typedef bool (*CompareFunc)(void*, void*);

//a function to write the data to the pipe
typedef void (*WriteFunc)(void*, int);

// Create a new queue with the given capacity
Queue queue_create(int capacity);

// Get the number of elements in the queue
int queue_size(Queue q);

// Add an element to the queue if it is not full and signal the consumer
void queue_produce(Queue q, void* data);

// Remove an element from the queue and signal the producer
void* queue_consume(Queue q);

// Remove an element from the queue using the provided function
void* queue_remove_by(Queue q, CompareFunc compare_func, void* arg);

// Send all the elements in the queue using the provided function
void queue_send_all(Queue q, WriteFunc write_func, int clientSocket) ;

// Destroy the queue and free all elements using the provided function
void queue_destroy(Queue q);

// Notify all Commanders in queue that server exits before executing their job
// and free all elements using the provided function
void queue_delete_and_notify_all(Queue q, FreePtr free_func);

#endif