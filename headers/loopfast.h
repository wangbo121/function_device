/*
 * loopfast.h
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#ifndef HEADERS_LOOPFAST_H_
#define HEADERS_LOOPFAST_H_

#include <semaphore.h>

#define MAX_LOOPFAST_TICK 10000

extern sem_t sem_loopfast;

void loopfast(void);

#endif /* HEADERS_LOOPFAST_H_ */
