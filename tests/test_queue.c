/*
 * test_queue.c
 *
 *  Created on: Mar 27, 2010
 *      Author: Freifeld Royi
 */

#ifndef TEST_QUEUE_C_
#define TEST_QUEUE_C_

#include "include/queue.h"
#include "include/test_queue.h"
#include <stdio.h>

/**
 *	should create a a new queue with all fields nullified
 *	and release the memory allocated
 *	error numbers:
 *	0 = OK
 *	-1 = head is not null
 *	-2 = tail is not null
 *	-3 = size is not 0
 *	-4 = memory was not freed
 */
int test_creation()
{
	int toReturn = 0;
	queue_t* t_qu = queue_init();

	if (QUEUE_HEAD(t_qu) != NULL)
		toReturn = err_number(toReturn, -1);
	if (QUEUE_TAIL(t_qu) != NULL)
		toReturn = err_number(toReturn, -2);
	if (QUEUE_SIZE(t_qu) != 0)
		toReturn = err_number(toReturn, -3);

	toReturn = err_number(toReturn, queue_destroy(t_qu));

	return toReturn;
}

int test_node_creation()
{
	int toReturn = 0;
	node_t* t_nd = node_init();

	if (NODE_DATA(t_nd) != NULL)
		toReturn = err_number(toReturn, -1);
	if (NODE_NEXT(t_nd) != NULL)
		toReturn = err_number(toReturn, -2);

	toReturn = err_number(toReturn, node_destroy(t_nd,NULL));

	return toReturn;
}

/**
 * should create new queue, insert a new node, increment the size
 * of the queue, remove the node, destroy the node (manually),
 * decrement the size and destroy the queue
 * error code:
 * -1 = node data isn't null
 * -2 = node next isn't null
 */
int test_queue_pop()
{
	int toReturn = 0;
	queue_t* t_qu = queue_init();
	node_t* t_nd1 = node_init();
	node_t* t_nd2 = node_init();
	node_t* t_nd3 = node_init();

	//push testing1
	queue_push(t_qu,t_nd1);

	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd1)
		toReturn = err_number(toReturn, -1);
	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd1)
		toReturn = err_number(toReturn, -2);
	if (QUEUE_SIZE(t_qu) != 1)
		toReturn = err_number(toReturn, -3);

	//push testing2
	queue_push(t_qu,t_nd2);

	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd1)
		toReturn = err_number(toReturn, -1);
	if (NODE_DATA(QUEUE_TAIL(t_qu)) != t_nd2)
		toReturn = err_number(toReturn, -2);
	if (QUEUE_SIZE(t_qu) != 2)
		toReturn = err_number(toReturn, -3);
	if (NODE_DATA(NODE_NEXT(QUEUE_HEAD(t_qu))) != t_nd2)
		toReturn = err_number(toReturn, -4);

	//push testing3
	queue_push(t_qu,t_nd3);

	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd1)
		toReturn = err_number(toReturn, -1);
	if (NODE_DATA(QUEUE_TAIL(t_qu)) != t_nd3)
		toReturn = err_number(toReturn, -2);
	if (QUEUE_SIZE(t_qu) != 3)
		toReturn = err_number(toReturn, -3);
	if (NODE_DATA(NODE_NEXT(QUEUE_HEAD(t_qu))) != t_nd2) //NODE_NEXT(QUEUE_HEAD(t_qu))
		toReturn = err_number(toReturn, -4);
	if (NODE_DATA(NODE_NEXT(NODE_NEXT(QUEUE_HEAD(t_qu)))) != t_nd3) //NODE_NEXT(NODE_NEXT(QUEUE_HEAD(t_qu)))
		toReturn = err_number(toReturn, -4);

	//pop testing1
	queue_pop(t_qu);

	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd2)
		toReturn = err_number(toReturn, -5);
	if (NODE_DATA(QUEUE_TAIL(t_qu)) != t_nd3)
		toReturn = err_number(toReturn, -6);
	if (QUEUE_SIZE(t_qu) != 2)
		toReturn = err_number(toReturn, -7);

	//pop testing2
	queue_pop(t_qu);

	if (NODE_DATA(QUEUE_HEAD(t_qu)) != t_nd3)
		toReturn = err_number(toReturn, -5);
	if (NODE_DATA(QUEUE_TAIL(t_qu)) != t_nd3)
		toReturn = err_number(toReturn, -6);
	if (QUEUE_SIZE(t_qu) != 1)
		toReturn = err_number(toReturn, -7);

	//pop testing3
	queue_pop(t_qu);

	if (QUEUE_HEAD(t_qu) != NULL)
		toReturn = err_number(toReturn, -5);
	if (QUEUE_TAIL(t_qu) != NULL)
		toReturn = err_number(toReturn, -6);
	if (QUEUE_SIZE(t_qu) != 0)
		toReturn = err_number(toReturn, -7);

	toReturn = err_number(toReturn, queue_destroy(t_qu));
	toReturn = err_number(toReturn, node_destroy(t_nd1,NULL));
	toReturn = err_number(toReturn, node_destroy(t_nd2,NULL));
	toReturn = err_number(toReturn, node_destroy(t_nd3,NULL));

	return toReturn;
}

int test_node_delete()
{
	int toReturn = 0;
	queue_t* t_toDestroy = queue_init();
	node_t* t_toAdd = node_init();
	node_t* t_nd = node_init();

	node_set_data(t_nd,t_toDestroy);

	toReturn = err_number(toReturn, queue_push(t_toDestroy, t_toAdd));
	toReturn = err_number(toReturn, node_destroy(queue_pop(t_toDestroy), NULL));
	toReturn = err_number(toReturn, node_destroy(t_nd, &queue_destroy));

	return toReturn;
}

void run_tests ()
{
	printf("test initialize result is: %d\n",test_creation()); // should be 0
	printf("test node creation result is: %d\n",test_node_creation()); // should be 1
	printf("test pop result is: %d\n",test_queue_pop()); // should be 111
	printf("test node destruction is: %d\n",test_node_delete()); //should be 110
}

int err_number(int toReturn, int toAdd)
{
	toReturn *= 10;
	if (toReturn == 0)
		toReturn = toAdd;
	else
		toReturn += toAdd;

	return toReturn;
}

#endif //TEST_QUEUE_C_
