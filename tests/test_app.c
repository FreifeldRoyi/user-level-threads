/*
 * test_app.h
 *
 *  Created on: Apr 16, 2010
 *      Author: freifeldroyi
 */

#include "include/test_app.h"
#include "../include/app_main.h"

void run_app_tests()
{
	printf("hello");
	int i = app_main(0,NULL);
	printf("%d",i);
}
