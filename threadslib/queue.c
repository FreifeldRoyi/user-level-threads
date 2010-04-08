/*
 * queue.c
 *
 *  Created on: March 26, 2010
 *      Author: Freifeld Royi
 */
#include "include/queue.h"
#include <assert.h>

node_t* node_init()
{
	node_t* toReturn = (node_t*)malloc(sizeof(node_t));
	assert(toReturn != NULL);
	NODE_DATA(toReturn) = NULL;
	NODE_NEXT(toReturn) = NULL;

	return toReturn;
}

int node_set_data(node_t* nd, void* data)
{
	int toReturn = -1;

	if (nd != NULL)
	{
		NODE_DATA(nd) = data;
		toReturn = 0;
	}

	return toReturn;
}

int node_set_next(node_t* nd, node_t* node_next)
{
	int toReturn = -1;

	if (nd != NULL)
	{
		NODE_NEXT(nd) = node_next;
		toReturn = 0;
	}
	else
		toReturn = 1;

	return toReturn;
}

int node_destroy(node_t* nd, int(*destfunc)(void*))
{
	int toReturn = 0;

	if (destfunc != NULL)
		(*destfunc)(NODE_DATA(nd));
	else
		toReturn = 1;

	free(nd);

	return toReturn;
}

queue_t* queue_init()
{
	queue_t* toReturn = (queue_t*)malloc(sizeof(queue_t));
	assert(toReturn != NULL);
	QUEUE_HEAD(toReturn) = NULL;
	QUEUE_TAIL(toReturn) = NULL;
	QUEUE_SIZE(toReturn) = 0;

	return toReturn;
}

void* queue_pop(queue_t* item)
{
	node_t* nodeToPop = NULL;
	void* toReturn = NULL;

	if (QUEUE_SIZE(item) > 0)
	{
		nodeToPop = QUEUE_HEAD(item);

		if (nodeToPop == QUEUE_TAIL(item))
			QUEUE_TAIL(item) = NULL;

		QUEUE_HEAD(item) = NODE_NEXT(QUEUE_HEAD(item));
		toReturn = NODE_DATA(nodeToPop);
	}

	queue_dec_size(item);

	return toReturn;
}

int queue_push(queue_t* queue, void* item)
{
	int toReturn = 0;
	node_t* toPush = node_init();
	node_set_data(toPush, item);

	toReturn = node_set_next(QUEUE_TAIL(queue), toPush);
	if (toReturn == 1)
	{
		QUEUE_HEAD(queue) = toPush;
	}
	QUEUE_TAIL(queue) = toPush;
	queue_inc_size(queue);

	return toReturn;
}

int queue_size(queue_t* item)
{
	return QUEUE_SIZE(item);
}

int queue_dec_size(queue_t* item)
{
	if (QUEUE_SIZE(item) > 0)
		--QUEUE_SIZE(item);

	return QUEUE_SIZE(item);
}

int queue_inc_size(queue_t* item)
{
	return ++QUEUE_SIZE(item);
}

int queue_destroy(queue_t* dest)
{
	int toReturn = -1;

	if (QUEUE_SIZE(dest) == 0)
	{
		free(dest);
		toReturn = 0;
	}

	return toReturn;
}



