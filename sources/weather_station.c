/*
 * weather_station.c
 *
 *  Created on: Oct 14, 2016
 *      Author: wangbo
 */


#include <stdio.h>
#include <stdlib.h>//atoi
#include <string.h>

/*转换int或者short的字节顺序，该程序arm平台为大端模式，地面站x86架构为小端模式*/
#include <byteswap.h>

/*open() close()所需头文件*/
#include <unistd.h>

#include "uart.h"
#include "utilityfunctions.h"

#include "weather_station.h"

#define AWS_RECV_BUF_LEN 512

#define AWS_RECV_HEAD1    0
#define AWS_RECV_LEN      1
#define AWS_RECV_DATA     2
#define AWS_RECV_CHECKSUM 3


T_AWS read_aws;

static int aws_recv_state = 0;
static int aws_uart_init_real(struct T_UART_DEVICE_PROPERTY *ptr_uart_device_property);
static int decode_aws_data(T_AWS aws,char *buf);

int aws_uart_init(unsigned int uart_num)
{
	uart_device_property.uart_num=uart_num;
	uart_device_property.baudrate=UART_AWS_BAUD;
	uart_device_property.databits=UART_AWS_DATABITS;
	uart_device_property.parity=UART_AWS_PARITY;
	uart_device_property.stopbits=UART_AWS_STOPBITS;

	aws_uart_init_real(&uart_device_property);

	return 0;
}

static int aws_uart_init_real(struct T_UART_DEVICE_PROPERTY *ptr_uart_device_property)
{
	if(-1!=(uart_fd[ptr_uart_device_property->uart_num]=open_uart_dev(uart_fd[ptr_uart_device_property->uart_num], ptr_uart_device_property->uart_num)))
	{
		printf("uart_fd[UART_AWS]=%d\n",uart_fd[ptr_uart_device_property->uart_num]);
		set_uart_opt( uart_fd[ptr_uart_device_property->uart_num], \
					  ptr_uart_device_property->baudrate,\
					  ptr_uart_device_property->databits,\
					  ptr_uart_device_property->parity,\
					  ptr_uart_device_property->stopbits);

		uart_device_pthread(ptr_uart_device_property->uart_num);
	}

	return 0;
}

int read_aws_data(T_AWS *ptr_read_aws,unsigned char *buf, unsigned int len)
{
	static unsigned char _buffer[AWS_RECV_BUF_LEN];

	//static unsigned char _pack_recv_type;
	//static int _pack_recv_cnt = 0;
	static int _pack_recv_len = 0;
	static unsigned char _pack_recv_buf[AWS_RECV_BUF_LEN];
	static int _pack_buf_len = 0;
	static unsigned char _checksum = 0;
	//unsigned char _sysid;

	int _length;
	int i = 0;
	unsigned char c;

	memcpy(_buffer, buf, len);

	/*显示收到的数据*/
#if 0
	printf("aws data buf=\n");
	for(i=0;i<len;i++)
	{
		printf("%c",buf[i]);
	}
	printf("\n");
#endif

	_length = len;

	for (i = 0; i<_length; i++)
	{
		c = _buffer[i];
		switch (aws_recv_state)
		{
		case AWS_RECV_HEAD1:
			if (c == 0x7E)
			{
				aws_recv_state = AWS_RECV_DATA;
				_pack_recv_len = 69;//气象站：数据+帧头0x7E+校验2 一共有72个字节
			}
			else
			{
				aws_recv_state = AWS_RECV_HEAD1;
			}
			_pack_buf_len = 0;

			break;
		case AWS_RECV_DATA:
			_pack_recv_buf[_pack_buf_len] = c;
			//_checksum += c;//根据协议要求这里应该添加crc校验的
			_pack_buf_len++;
			if (_pack_buf_len >= _pack_recv_len)
			{
				aws_recv_state = AWS_RECV_CHECKSUM;
			}
			break;
		case AWS_RECV_CHECKSUM:
			//if (_checksum == c)//这里先不判断crc校验了吧
			if(1)
			{

				//处理气象站数据
				printf("_pack_recv_len=%d\n",_pack_recv_len);
				printf("read_aws占用%d个字节\n",sizeof(read_aws));
				//memcpy(&read_aws, _pack_recv_buf_frame, _pack_recv_len);
				decode_aws_data(read_aws,(char *)_pack_recv_buf);


			}
			else
			{
				aws_recv_state = 0;
			}
		}
	}

	return 0;
}

#define position0  0
#define position1  1//年
#define position2  5//月
#define position3  7//日
#define position4  9//时
#define position5  11//分
#define position6  13//秒
#define position7  15//东 或者 西 E，W
#define position8  16//经度
#define position9  26//北 或者 南 N，S
#define position10 27//纬度
#define position11 36//高度
#define position12 38//温度
#define position13 40//湿度
#define position14 42//风速
#define position15 44//风向
#define position16 46//气压
#define position17 48//辐射1
#define position18 50//辐射2
#define position19 52//盐海温
#define position20 56//电导率
#define position21 60//海温1
#define position22 62//海温2
#define position23 64//海温3
#define position24 66//海温4
#define position25 68//海温5
#define position26 70//校验

int decode_aws_data(T_AWS aws,char *buf)
{
	unsigned char size_byte;
	unsigned char index;
	//unsigned char temp[20];
	char temp[20];
	char buf_frame[125]={0};

	buf_frame[0]=0x7E;
	memcpy(&buf_frame[1],buf,69);//需要69个字节

	//日期：年
	index=position1;
	size_byte=position2-position1;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.year=atoi(temp);

	//月
	index=position2;
	size_byte=position3-position2;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.month=atoi(temp);

	//日
	index=position3;
	size_byte=position4-position3;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.day=atoi(temp);

	//时间 时
	index=position4;
	size_byte=position5-position4;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.hour=atoi(temp);

	//分
	index=position5;
	size_byte=position6-position5;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.minute=atoi(temp);

	//秒
	index=position6;
	size_byte=position7-position6;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.second=atoi(temp);

	//东 或者 西
	index=position7;
	aws.east_west=buf_frame[index];

	index=position8;
	size_byte=position9-position8;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.longitude=atoi(temp);

	index=position9;
	aws.north_south=buf_frame[index];

	index=position10;
	size_byte=position11-position10;
	memcpy(temp,buf_frame+index,size_byte);
	temp[size_byte]='\0';
	aws.latitude=atoi(temp);
/*************************************************************/
	//高度
	index=position11;
	size_byte=position12-position11;
	memcpy(&aws.height,buf_frame+index,size_byte);
	aws.height=__bswap_16(aws.height);
	//温度
	index=position12;
	size_byte=position13-position12;
	memcpy(&aws.temperature,buf_frame+index,size_byte);
	aws.temperature=__bswap_16(aws.temperature);
	//湿度
	index=position13;
	size_byte=position14-position13;
	memcpy(&aws.humidity,buf_frame+index,size_byte);
	aws.humidity=__bswap_16(aws.humidity);
	//风速
	index=position14;
	size_byte=position15-position14;
	memcpy(&aws.wind_speed,buf_frame+index,size_byte);
	aws.wind_speed=__bswap_16(aws.wind_speed);
	//风向
	index=position15;
	size_byte=position16-position15;
	memcpy(&aws.wind_dir,buf_frame+index,size_byte);
	aws.wind_dir=__bswap_16(aws.wind_dir);
	//气压
	index=position16;
	size_byte=position17-position16;
	memcpy(&aws.air_press,buf_frame+index,size_byte);
	aws.air_press=__bswap_16(aws.air_press);
	//辐射1
	index=position17;
	size_byte=position18-position17;
	memcpy(&aws.radiation1,buf_frame+index,size_byte);
	aws.radiation1=__bswap_16(aws.radiation1);
	//辐射2
	index=position18;
	size_byte=position19-position18;
	memcpy(&aws.radiation2,buf_frame+index,size_byte);
	aws.radiation2=__bswap_16(aws.radiation2);
	//盐海温
	index=position19;
	size_byte=position20-position19;
	memcpy(&aws.salt_sea_temp,buf_frame+index,size_byte);
	aws.salt_sea_temp=__bswap_32(aws.salt_sea_temp);
	//电导率
	index=position20;
	size_byte=position21-position20;
	memcpy(&aws.conductivity,buf_frame+index,size_byte);
	aws.conductivity=__bswap_32(aws.conductivity);
	//海温1
	index=position21;
	size_byte=position22-position21;
	memcpy(&aws.sea_temp1,buf_frame+index,size_byte);
	aws.sea_temp1=__bswap_16(aws.sea_temp1);
	//海温2
	index=position22;
	size_byte=position23-position22;
	memcpy(&aws.sea_temp2,buf_frame+index,size_byte);
	aws.sea_temp2=__bswap_16(aws.sea_temp2);
	//海温3
	index=position23;
	size_byte=position24-position23;
	memcpy(&aws.sea_temp3,buf_frame+index,size_byte);
	aws.sea_temp3=__bswap_16(aws.sea_temp3);
	//海温4
	index=position24;
	size_byte=position25-position24;
	memcpy(&aws.sea_temp4,buf_frame+index,size_byte);
	aws.sea_temp4=__bswap_16(aws.sea_temp4);
	//海温5
	index=position25;
	size_byte=position26-position25;
	memcpy(&aws.sea_temp5,buf_frame+index,size_byte);
	aws.sea_temp5=__bswap_16(aws.sea_temp5);

	return 0;
}


int aws_uart_close(unsigned int uart_num)
{
	close(uart_fd[uart_num]);

	return 0;
}
