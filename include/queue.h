/*
 * queue.h
 *
 *  Created on: Mar 26, 2010
 *      Author: Freifeld Royi
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdlib.h>

// queue_t macros
#define QUEUE(x) ((queue_t *) (x))
#define QUEUE_HEAD(x) QUEUE((x)) -> head
#define QUEUE_TAIL(x) QUEUE((x)) -> tail
#define QUEUE_SIZE(x) QUEUE((x)) -> size

// node_t macros
#define NODE(x) ((node_t *) (x))
#define NODE_DATA(x) NODE((x)) -> data
#define NODE_NEXT(x) NODE((x)) -> next

/**
 * Defining a new queue node
 */
typedef struct queue_node
{
	void* data;
	struct queue_node* next;
} node_t;

/**
 * Defining a new queue data structure.
 * Holds a pointer to the head, and tail.
 * In addition holds the number of items.
 */
typedef struct queue
{
	node_t* head;
	node_t* tail;
	int size;
} queue_t;

/**
 * Node initialization
 * returns a pointer the new node with all fields nullified
 */
node_t* node_init();

/**
 * Setting node's data. Use this method in order to set the node's
 * data. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and -1
 * 0 will be returned if everything is o.k
 */
int node_set_data(node_t* nd, void* data);

/**
 * Setting node's next. Use this method in order to set the node's
 * next. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and 1 will be returned
 * 0 will be returned if everything is o.k
 * -1 if insert fails
 */
int node_set_next(node_t* nd, node_t* node_next);

/**
 * will free all memory allocated to the node
 * where destfunc is the data's free function.
 * if destfunc is NULL, the data won't be deleted,
 * and it's up to the user to delete it and 1 will be returned.
 * in case of successful deletion 0 will be returned
 * PAY ATTENTION! it's recommended to delete the data
 * before using this function
 */
int node_destroy(node_t* nd, int(*destfunc)(void*));

/**
 * Queue initialization.
 * Returns a pointer to the head of the queue.
 */
queue_t* queue_init();

/**
 * Returns the head of the queue's data and dequeues the node
 */
void* queue_pop(queue_t* item);

/**
 * Inserts a new queue item to the end of the queue
 * Returns 1 if item is NULL
 * 0 if insert succeeds or -1 if insert fails
 */
int queue_push(queue_t* queue, void* item);

/**
 * Returns the number of items in the queue
 *
 */
int queue_size(queue_t* item);

/**
 * decrement the queue's size.
 * If queue's size < 0 then nothing will be changed (in case of manually
 * changed size) use this function to decrement the quese's size,
 * don't do it manually.
 * returns the new queue size
 */
int queue_dec_size(queue_t* item);

/**
 * increment the queue's size, don't do it manually
 * returns the new queue size.
 */
int queue_inc_size(queue_t* item);

/**
 * Will free all memory allocated in the queue.
 * Returns 0 if the queue is empty and all memory was free
 * or -1 if freeing the memory fails or the queue isn't empty
 */
int queue_destroy(queue_t* dest);

#endif /* QUEUE_H_ */
