/*
 * loopfast.c
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#include <stdio.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopfast.h"

sem_t sem_loopfast;

void loopfast(void)
{
	printf("Enter the loopfast...\n");

	while(main_task.loopfast_permission)
	{
		sem_wait(&sem_loopfast);     /*等待信号量*/

		/*
		 * 需要在快循环执行的程序，写在这里
		 */

		main_task.loopfast_cnt++;

		if(main_task.loopfast_cnt>=MAX_LOOPFAST_TICK)
		{
			main_task.loopfast_cnt=0;
		}
	}
}
