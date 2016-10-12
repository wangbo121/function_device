/*
 * global.h
 *
 *  Created on: Oct 12, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_GLOBAL_H_
#define HEADERS_GLOBAL_H_

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct
{
	unsigned char bool_get_rocket_command;
	unsigned char bool_get_rocket_ari_sounding;

	unsigned char bool_rocket_request_attitude;

}T_GLOBAL_BOOL;


extern T_GLOBAL_BOOL global_bool;


#endif /* HEADERS_GLOBAL_H_ */
