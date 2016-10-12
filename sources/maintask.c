/*
 * maintask.c
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopfast.h"
#include "loopslow.h"
#include "loopforever.h"

#include "utilityfunctions.h"

/*static variable*/
static float maintask_start2current_time;

/*extern variable*/
struct T_MAIN_TASK main_task;

void maintask()
{
		//int seconds=1;
	    //int mseconds=0;/*微秒，microsecond is Not millisecond*/
		int seconds=0;
		int mseconds=MAINTASK_TICK_TIME*(1e3);/*每个tick为20毫秒，也就是20000微秒,mseconds是微秒的意思*/

        struct timeval temp;

        pthread_t loopfast_pthrd = 0;
        pthread_t loopslow_pthrd = 0;
        pthread_t loopforever_pthrd = 0;
        int ret=0;

        static int sem_loopfast_cnt;
        static int sem_loopslow_cnt;
        static int sem_loopforever_cnt;

        printf("Enter the maintask...\n");

        init_maintask();

        /*
         * 初始化快循环信号量
         */
        sem_init(&sem_loopfast,0,1);/*初始化时，信号量为1*/
        ret = pthread_create (&loopfast_pthrd,          //线程标识符指针
							  NULL,                     //默认属性
							  (void *)loopfast,         //运行函数
							  NULL);                    //无参数
	    if (0 != ret)
	    {
	       perror ("pthread create error\n");
	    }

	    /*
	     * 初始化慢循环信号量
	     */
	    sem_init(&sem_loopslow,0,1);
        ret = pthread_create (&loopslow_pthrd,          //线程标识符指针
							  NULL,                     //默认属性
							  (void *)loopslow,         //运行函数
							  NULL);                    //无参数

	    if (0 != ret)
	    {
	       perror ("pthread create error\n");
	    }

	    /*
		 * 初始化永久循环信号量
		 */
		sem_init(&sem_loopforever,0,1);/*初始化时，信号量为1*/
		ret = pthread_create (&loopforever_pthrd,          //线程标识符指针
							  NULL,                     //默认属性
							  (void *)loopforever,         //运行函数
							  NULL);                    //无参数
		if (0 != ret)
		{
		   perror ("pthread create error\n");
		}

	    /*
	     * 开始maintask线程的主循环，相当于最小的tick周期时间
	     */
        while (1)
        {
        	temp.tv_sec = seconds;
        	temp.tv_usec = mseconds;
        	select(0, NULL, NULL, NULL, &temp);

		    main_task.maintask_cnt++;

		    /*loopfast 快循环*/
		    if(0 == main_task.maintask_cnt%LOOP_FAST_TICK)
		    {

		    	/*
		    	 * 如果直接sem_post的话，是计数器的信号量，这个会累计
		    	 * 但是需要的是在执行一次后，就把之前的清零，也就是让该信号量为2值信号量
		    	 */
		    	//sem_post (&sem_loopfast);        /*释放信号量*/
				while(!sem_getvalue(&sem_loopfast,&sem_loopfast_cnt))
				{
					if(sem_loopfast_cnt>=1)
					{
						sem_wait(&sem_loopfast);
					}
					else
					{
						/*
						 * 释放信号量
						 * 在给出信号量之前，一定保证信号量为0
						 */
						sem_post (&sem_loopfast);
						break;
					}
				}
		    }

		    /*loopslow 慢循环*/
			if(0 == main_task.maintask_cnt%LOOP_SLOW_TICK)
			{
				while(!sem_getvalue(&sem_loopslow,&sem_loopslow_cnt))
				{
					if(sem_loopslow_cnt>=1)
					{
						sem_wait(&sem_loopslow);
					}
					else
					{
						sem_post (&sem_loopslow);        /*释放信号量*/
						break;
					}
				}

				maintask_start2current_time=clock_gettime_s();
				printf("系统从开启到当前时刻运行的时间%f[s]\n",maintask_start2current_time);
			}

		    /*loopforever 循环*/
			if(0 == main_task.maintask_cnt%LOOP_FOREVER_TICK)
			{
				while(!sem_getvalue(&sem_loopforever,&sem_loopforever_cnt))
				{
					if(sem_loopforever_cnt>=1)
					{
						sem_wait(&sem_loopforever);
					}
					else
					{
						sem_post (&sem_loopforever);
						break;
					}
				}
			}

			if(main_task.maintask_cnt>=MAX_MAINTASK_TICK)
			{
				main_task.maintask_cnt=0;
			}
        }

	    pthread_join (loopfast_pthrd, NULL);
	    pthread_join (loopslow_pthrd, NULL);
	    pthread_join (loopforever_pthrd, NULL);

        return ;
}

void init_maintask(void)
{
	main_task.loopfast_permission=1;
	main_task.loopslow_permission=1;
	main_task.loopforever_permission=1;
}

void stop_alltask(void)
{
	main_task.loopfast_permission=0;
	main_task.loopslow_permission=0;
	main_task.loopforever_permission=0;
}

