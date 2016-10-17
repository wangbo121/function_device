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


//#define __UART_ROCKET_
#define __UART_AWS_

/*保存文件的文件名和文件句柄（文件描述符fd）*/
#define ROCKET_AIR_SOUNDING_FILE "rocket_air.txt"
#define AIR_WEATHER_STATION "weather_station.txt"
extern int fd_rocket_air_sounding_log;
extern int fd_air_weather_station_log;




#endif /* HEADERS_GLOBAL_H_ */
