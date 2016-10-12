/*
 * loopforever.h
 *
 *  Created on: Aug 19, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_LOOPFOREVER_H_
#define HEADERS_LOOPFOREVER_H_


#include <semaphore.h>

#define MAX_LOOPFOREVER_TICK 10000

extern sem_t sem_loopforever;

void loopforever(void);


#endif /* HEADERS_LOOPFOREVER_H_ */
