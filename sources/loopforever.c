/*
 * loopforever.c
 *
 *  Created on: Aug 19, 2016
 *      Author: wangbo
 */
#include <stdio.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopforever.h"

sem_t sem_loopforever;

void loopforever(void)
{
	printf("Enter the loopforever...\n");

	while(main_task.loopforever_permission)
	{
		sem_wait(&sem_loopforever);     /*等待信号量*/

		/*
		 * 需要在永久循环执行的程序，写在这里
		 */

		main_task.loopforever_cnt++;

		if(main_task.loopforever_cnt>=MAX_LOOPFOREVER_TICK)
		{
			main_task.loopforever_cnt=0;
		}
	}
}
