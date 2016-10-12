/*
 * loopslow.c
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopslow.h"

#include "utilityfunctions.h"

sem_t sem_loopslow;

void loopslow(void)
{
	printf("Enter the loopslow...\n");

	while(main_task.loopslow_permission)
	{

		main_task.loopslow_cnt++;

		sem_wait(&sem_loopslow);     /*等待信号量*/

		/*
		 * 需要在慢循环执行的程序，写在这里
		 */
		printf("hello i am loopslow!!!\n");

		if(main_task.loopslow_cnt>=MAX_LOOPSLOW_TICK)
		{
			main_task.loopslow_cnt=0;
		}
	}
}
