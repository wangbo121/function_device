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
	unsigned char bool_get_rocket_air_sounding;

	unsigned char bool_rocket_request_attitude;

}T_GLOBAL_BOOL;

extern T_GLOBAL_BOOL global_bool;

#define __UART_ROCKET_
#define __UART_AWS_
#define __UART_MODBUS_
#define __UDP_

/*保存文件的文件名和文件句柄（文件描述符fd）*/
#define ROCKET_ROCKET_AIR_SOUNDING_FILE "rocket_air.txt"
#define AIR_WEATHER_STATION "weather_station.txt"
extern int fd_rocket_air_sounding_log;
extern int fd_air_weather_station_log;

//#define UART_AWS       "/dev/ttyO3"
//#define UART_ROCKET    "/dev/ttyO4"//这是文档上的，但是却是错误的
#define UART_AWS       "/dev/ttyO4"
#define UART_ROCKET    "/dev/ttyO3"
#define UART_MODBUS "/dev/ttyO1"

#define IP_MASTER "10.10.80.1"//给主控板发送
//#define IP_MASTER "10.10.80.2"//给自己发送
#define IP_SLAVER "10.10.80.2"
#define UDP_SERVER_PORT 7500

#endif /* HEADERS_GLOBAL_H_ */
