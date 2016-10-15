/*
 * weather_station.h
 *
 *  Created on: Oct 14, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_WEATHER_STATION_H_
#define HEADERS_WEATHER_STATION_H_

#define UART_AWS 6///dev/ttyUSB0
//#define UART_AWS 7///dev/ttyUSB1
#define UART_AWS_BAUD 9600
#define UART_AWS_DATABITS 8 //8 data bit
#define UART_AWS_STOPBITS 1 //1 stop bit
#define UART_AWS_PARITY 0 //no parity
#if 0
//52 bytes
typedef struct
{
    //1 bytes
	unsigned char frame_head;

	//4 bytes
	unsigned short year;
	unsigned char month;
	unsigned char day;

	//3 bytes
	unsigned char hour;
	unsigned char minute;
	unsigned char second;

	//10 bytes
	unsigned char east_west;
	unsigned int longitude;
	unsigned char north_south;
	unsigned int latitude;

	//8*2+2*4+5*2 =34 bytes
	unsigned short height;
	unsigned short temperature;
	unsigned short humidity;//湿度
	unsigned short wind_speed;
	unsigned short wind_dir;
	unsigned short air_press;
	unsigned short radiation1;//辐射
	unsigned short radiation2;
	unsigned int salt_sea_temp;//盐海温
	unsigned int conductivity;//导电率
	unsigned short sea_temp1;//海温
	unsigned short sea_temp2;
	unsigned short sea_temp3;
	unsigned short sea_temp4;
	unsigned short sea_temp5;
}T_AWS;
#else
typedef struct
{
    //尽量字节字节放在一起或者凑成4的倍数，因为有对齐的影响导致sizeof(T_AWS)的个数不对
    //3 bytes
    unsigned char frame_head;
    unsigned char month;
    unsigned char day;
    //3 bytes
    unsigned char hour;
    unsigned char minute;
    unsigned char second;

    unsigned char east_west;
    unsigned char north_south;

    //2 bytes
    unsigned short year;
    unsigned short height;
    unsigned short temperature;
    unsigned short humidity;//湿度

    unsigned short wind_speed;
    unsigned short wind_dir;
    unsigned short air_press;
    unsigned short radiation1;//辐射

    unsigned short radiation2;
    unsigned short sea_temp1;//海温
    unsigned short sea_temp2;
    unsigned short sea_temp3;

    unsigned short sea_temp4;
    unsigned short sea_temp5;

    unsigned int salt_sea_temp;//盐海温
    unsigned int conductivity;//导电率
    unsigned int longitude;
    unsigned int latitude;
}T_AWS;
#endif



//AWS=air weather station 气象站
extern T_AWS read_aws;


int aws_uart_init(unsigned int uart_num);
/*
 * 获取从aws传回来的数据
 */
int read_aws_data(T_AWS *ptr_read_aws,unsigned char *buf, unsigned int len);
/*
 * 自驾仪给火箭设备发送命令
 */
int write_aws_data(T_AWS *ptr_write_aws);

int aws_uart_close(unsigned int uart_num);


#endif /* HEADERS_WEATHER_STATION_H_ */
