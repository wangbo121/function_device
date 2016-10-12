/*
 * loopslow.h
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#ifndef HEADERS_LOOPSLOW_H_
#define HEADERS_LOOPSLOW_H_

#include <semaphore.h>

#define MAX_LOOPSLOW_TICK 10000

extern sem_t sem_loopslow;

void loopslow(void);

#endif /* HEADERS_LOOPSLOW_H_ */
