/*
 * rocket.h
 *
 *  Created on: Oct 12, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_ROCKET_H_
#define HEADERS_ROCKET_H_

#define pilot_device_address 0x01
#define rocket_device_address 0x02

//#define UART_ROCKET 6///dev/ttyUSB0
#define UART_ROCKET 7///dev/ttyUSB1
#define UART_ROCKET_BAUD 9600
#define UART_ROCKET_DATABITS 8 //8 data bit
#define UART_ROCKET_STOPBITS 1 //1 stop bit
#define UART_ROCKET_PARITY 0 //no parity

typedef struct
{
	unsigned char device_num;
	unsigned char function_num;
	unsigned char information;
	unsigned char invalid;//只是为了字节对齐，没有任何作用
}T_ROCKET;

typedef struct
{
    //这里我是知道我的笔记本和arm板的short都是2字节，int都是4字节，机器是2字节对齐的，所以可以用memcpy到某一个结构
    //如果不知道具体机器的short和int字节数，那就注意可能出现错误
	short temp;/*[*0.1度] -1000--1000*/

	unsigned short humidity;/*[*0.1度] 0--1000*/
	unsigned short air_pressure;/*[*0.1百帕] 3000--12000*/
	unsigned short wind_speed;/*[*0.1]*/
	unsigned short wind_dir;/*[*0.1]*/

	int longitude;/*[*10e-6度]*/
	int latitude;/*[*10e-6度]*/
	int height;/*[*0.1度] -100--100000*/

	unsigned char number;
	unsigned char invalid;//只是为了字节对齐，没有任何作用
}T_AIR_SOUNDING;

typedef struct
{
    int longitude;
    int latitude;

    short height;
    short pitch;
    short roll;
    short yaw;

}T_ROCKET_ATTITUDE;

extern T_ROCKET read_rocket;
extern T_ROCKET write_rocket;
extern T_AIR_SOUNDING air_sounding_rocket;
extern T_ROCKET_ATTITUDE rocket_attitude;

int rocket_uart_init(unsigned int uart_num);
/*
 * 获取从rocket传回来的数据
 */
int read_rocket_data(T_ROCKET *ptr_read_rocket,unsigned char *buf, unsigned int len);
/*
 * 自驾仪给火箭设备发送命令
 */
//int write_rocket_data(T_ROCKET *ptr_write_rocket);
int write_rocket_cmd(T_ROCKET *ptr_write_rocket);

int start_launch_rocket();
int write_rocket_attitude();

int rocket_uart_close(unsigned int uart_num);

#endif /* HEADERS_ROCKET_H_ */
