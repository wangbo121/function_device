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
#define AWS_RECV_CHECK_LOW 3
#define AWS_RECV_CHECK_HIGH 4

T_AWS read_aws;

static int aws_recv_state = 0;
static int aws_uart_init_real(struct T_UART_DEVICE_PROPERTY *ptr_uart_device_property);
static int decode_aws_data(T_AWS aws,char *buf);
static unsigned short get_crc16(unsigned char *buf, unsigned short len);

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
	//static unsigned char _checksum = 0;
	//unsigned char _sysid;
	static int _check_crc_low=0;
	static int _check_crc_high=0;
	unsigned char check_crc_buf[AWS_RECV_BUF_LEN]={0};

	unsigned short crc16;


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
				aws_recv_state = AWS_RECV_CHECK_LOW;
			}
			break;
		case AWS_RECV_CHECK_LOW:
		    _check_crc_low=c;
		    aws_recv_state = AWS_RECV_CHECK_HIGH;
		    break;
		case AWS_RECV_CHECK_HIGH:
		    _check_crc_high=c;

		    check_crc_buf[0]=0x7E;
		    memcpy(&check_crc_buf[1],_pack_recv_buf,_pack_recv_len);

		    crc16=get_crc16(check_crc_buf,1+_pack_recv_len);
		    if(_check_crc_high * 256 +_check_crc_low == crc16)
		    {
				//处理气象站数据
		        printf("正确接收气象站数据,准备处理\n");
				printf("_pack_recv_len=%d\n",_pack_recv_len);
				printf("read_aws占用%d个字节\n",sizeof(read_aws));
				//memcpy(&read_aws, _pack_recv_buf_frame, _pack_recv_len);
				decode_aws_data(read_aws,(char *)_pack_recv_buf);
			}
			else
			{
			    printf("crc校验不正确，重新接收\n");
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


unsigned short get_crc16(unsigned char *buf, unsigned short len)
{
    unsigned char all_crc_high[256] =
    {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
    };

    unsigned char all_crc_low[256] =
    {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40
    };

    unsigned char crc_high = 0xFF;
    unsigned char crc_low = 0xFF;
    unsigned index = 0;

    while (len--)
    {
        index = crc_high ^ *buf++;
        crc_high = crc_low ^ all_crc_high[index];
        crc_low = all_crc_low[index];
    }

    return (unsigned short)((unsigned short)crc_high << 8 | crc_low);
}
