/*
 * utilityfunctions.c
 *
 *  Created on: 2016年5月10日
 *      Author: wangbo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "utilityfunctions.h"

/*
 * 精确到毫秒的延时
 */
int delay_ms(int ms)
{
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = ms * 1000;
	select(0, NULL, NULL, NULL, &delay);

	return 0;
}

int sleep_ms(int ms)
{
	usleep(1000*ms);

	return 0;
}

/*
 * 获取系统时间
 * 从UTC(coordinated universal time)时间
 * 1970年1月1日00时00分00秒(也称为Linux系统的Epoch时间)到当前时刻的秒数
 */
float gettimeofday_s()
{
	struct timeval current_time;

	gettimeofday(&current_time,NULL);

	return (float)(current_time.tv_sec)*1+(float)(current_time.tv_usec)*1e-6;
}

float gettimeofday_ms()
{
	struct timeval current_time;

	gettimeofday(&current_time,NULL);

	return (float)(current_time.tv_sec)*1e3+(float)(current_time.tv_usec)*1e-3;
}

float gettimeofday_us()
{
	struct timeval current_time;

	gettimeofday(&current_time,NULL);

	return (float)(current_time.tv_sec)*1e6+(float)(current_time.tv_usec)*1;
}

struct timeval diff_gettimeofday(struct timeval start,struct timeval end)
{
	struct timeval temp;

	if ((end.tv_usec-start.tv_usec)<0)
	{
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_usec = 1e6+end.tv_usec-start.tv_usec;
	}
	else
	{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_usec = end.tv_usec-start.tv_usec;
	}

	return temp;

}

float diff_gettimeofday_value(float start,float end)
{
	return end-start;
}

/*
 * 获取以系统启动瞬间为基准的时间
 * long sys_clock_gettime (clockid_t which_clock, struct timespec *tp);
 */
/*
 *  且在编译链接时需加上 -lrt ;因为在librt中实现了clock_gettime函数。
 *  struct timespec ts;
 *  clock_gettime(CLOCK_MONOTONIC,ts);
 */
float clock_gettime_s()
{
	struct timespec current_time;

	clock_gettime(CLOCK_MONOTONIC, &current_time);

	return (float)(current_time.tv_sec)*1+(float)(current_time.tv_nsec)*1e-9;

}

struct timespec diff_clock(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec-start.tv_nsec)<0)
	{
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1e9+end.tv_nsec-start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}

	return temp;
}


