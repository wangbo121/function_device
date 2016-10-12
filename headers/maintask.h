/*
 * maintask.h
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#ifndef HEADERS_MAINTASK_H_
#define HEADERS_MAINTASK_H_

#define MAX_MAINTASK_TICK 10000

/*
 * maintask tick time [ms]
 */
#define MAINTASK_TICK_TIME 20

/*
 * execute every LOOP_FAST_TICK*MAINTASK_TICK_TIME ms
 */
#define LOOP_FAST_TICK 15
#define LOOP_SLOW_TICK 50
#define LOOP_FOREVER_TICK 2

struct T_MAIN_TASK
{
	unsigned char loopfast_permission;
	unsigned char loopslow_permission;
	unsigned char loopforever_permission;

	unsigned int loopfast_cnt;
	unsigned int loopslow_cnt;
	unsigned int loopforever_cnt;

	unsigned int maintask_cnt;
};

extern struct T_MAIN_TASK main_task;

void maintask(void);
void init_maintask(void);
void stop_alltask(void);

#endif /* HEADERS_MAINTASK_H_ */
