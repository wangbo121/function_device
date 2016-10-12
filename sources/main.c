/*
 * main.c
 *
 *  Created on: Oct 11, 2016
 *      Author: wangbo
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "maintask.h"

int main()
{
	/*thread: maintask*/
	pthread_t maintask_pthrd = 0;
	int ret=0;

	/*
	 * 主线程
	 * 主任务开始执行
	 */
	/*maintask thread*/
    ret = pthread_create (&maintask_pthrd,          //线程标识符指针
						  NULL,                     //默认属性
						  (void *)maintask,         //运行函数
						  NULL);                    //无参数
	if (0 != ret)
	{
	   perror ("pthread create error\n");
	}
   pthread_join (maintask_pthrd, NULL);

   return 0;
}



