/*
 * utilityfunctions.h
 *
 *  Created on: 2016年5月10日
 *      Author: wangbo
 */

#ifndef HEADERS_UTILITYFUNCTIONS_H_
#define HEADERS_UTILITYFUNCTIONS_H_

int delay_ms(int ms);
int sleep_ms(int ms);

float gettimeofday_s();
float gettimeofday_ms();
float gettimeofday_us();
struct timeval diff_gettimeofday(struct timeval start,struct timeval end);
float diff_gettimeofday_value(float start,float end);

/*
 * 获取以系统启动瞬间为基准的时间
 * long sys_clock_gettime (clockid_t which_clock, struct timespec *tp);
 * 且在编译链接时需加上 -lrt ;因为在librt中实现了clock_gettime函数。
 * struct timespec ts;
 * clock_gettime(CLOCK_MONOTONIC,ts);
 */
float clock_gettime_s();
struct timespec diff_clock(struct timespec start, struct timespec end);


#endif /* HEADERS_UTILITYFUNCTIONS_H_ */
