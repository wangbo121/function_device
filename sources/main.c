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

#include "global.h"

#include "maintask.h"
#include "loopfast.h"
#include "loopslow.h"
#include "loopforever.h"

#include "save_data.h"
#include "rocket.h"
#include "weather_station.h"
#include "utilityfunctions.h"
#include "udp.h"

/*定义保存文件描述符*/
int fd_rocket_air_sounding_log=0;
int fd_air_weather_station_log=0;

int main()
{
#ifdef __UART_ROCKET_
	rocket_uart_init();
	/*create log file*/
	fd_rocket_air_sounding_log=create_log_file(ROCKET_ROCKET_AIR_SOUNDING_FILE);

	//发射火箭指令
	//start_launch_rocket();
	write_rocket_attitude();
#endif
#ifdef __UART_AWS_
    aws_uart_init();
	/*create log file*/
	fd_air_weather_station_log=create_log_file(AIR_WEATHER_STATION);

#endif
#ifdef __UDP_
	open_udp_dev(IP_MASTER, UDP_SERVER_PORT, UDP_SERVER_PORT);
#endif
#if 1
    int seconds=0;
    int mseconds=MAINTASK_TICK_TIME*(1e3);/*每个tick为20毫秒，也就是20000微秒,mseconds是微秒的意思*/

    struct timeval maintask_tick;

    pthread_t loopfast_pthrd = 0;
    pthread_t loopslow_pthrd = 0;
    pthread_t loopforever_pthrd = 0;

    static int sem_loopfast_cnt;
    static int sem_loopslow_cnt;
    static int sem_loopforever_cnt;

    int ret=0;
    float system_running_time=0.0;

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

    sem_init(&sem_loopforever,0,1);/*初始化时，信号量为1*/
    ret = pthread_create (&loopforever_pthrd,          //线程标识符指针
                          NULL,                     //默认属性
                          (void *)loopforever,         //运行函数
                          NULL);                    //无参数
    if (0 != ret)
    {
       perror ("pthread create error\n");
    }

    while (1)
    {
        maintask_tick.tv_sec = seconds;
        maintask_tick.tv_usec = mseconds;
        select(0, NULL, NULL, NULL, &maintask_tick);

        main_task.maintask_cnt++;

        /*loopfast 快循环*/
        if(0 == main_task.maintask_cnt%LOOP_FAST_TICK)
        {
            while(!sem_getvalue(&sem_loopfast,&sem_loopfast_cnt))
            {
                if(sem_loopfast_cnt>=1)
                {
                    sem_wait(&sem_loopfast);
                }
                else
                {
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

            system_running_time=clock_gettime_s();
            printf("系统从开启到当前时刻运行的时间%f[s]\n",system_running_time);
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
    }//while结束

    pthread_join (loopfast_pthrd, NULL);
    pthread_join (loopslow_pthrd, NULL);
    pthread_join (loopforever_pthrd, NULL);

#else
    /*thread: maintask*/
    pthread_t maintask_pthrd = 0;
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
#endif

#ifdef __UART_ROCKET_
    close_log_file(fd_rocket_air_sounding_log);
    rocket_uart_close();
#endif
#ifdef __UART_AWS_
    close_log_file(fd_air_weather_station_log);
    aws_uart_close();
#endif

   return 0;
}



