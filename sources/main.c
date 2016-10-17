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
#include "save_data.h"

#include "rocket.h"
#include "weather_station.h"

/*定义保存文件描述符*/
int fd_rocket_air_sounding_log=0;
int fd_air_weather_station_log=0;

int main()
{
	/*thread: maintask*/
	pthread_t maintask_pthrd = 0;
	int ret=0;

#ifdef __UART_ROCKET_
	rocket_uart_init(UART_ROCKET);
	/*create log file*/
	fd_rocket_air_sounding_log=create_log_file(ROCKET_AIR_SOUNDING_FILE);

	//发射火箭指令
	start_launch_rocket();
#endif
#ifdef __UART_AWS_
	/*create log file*/
	fd_air_weather_station_log=create_log_file(AIR_WEATHER_STATION);
	aws_uart_init(UART_AWS);
#endif
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

#ifdef __UART_ROCKET_
    close_log_file(fd_rocket_air_sounding_log);
    rocket_uart_close(UART_ROCKET);
#endif
#ifdef __UART_AWS_
    close_log_file(fd_air_weather_station_log);
    aws_uart_close(UART_AWS);
#endif

   return 0;
}



