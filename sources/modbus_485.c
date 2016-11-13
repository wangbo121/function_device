/*
 * modbus_485.c
 *
 *  Created on: 2016年5月18日
 *      Author: wangbo
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "global.h"
#include "uart.h"
#include "modbus_power_supply.h"
#include "modbus_toggle_discharge.h"
#include "utilityfunctions.h"

#include "modbus_485.h"

#define MODBUS_RECV_BUF_LEN 512

#define MODBUS_RECV_DEVICE_ID  0
#define MODBUS_RECV_FUNC_CODE  1
#define MODBUS_RECV_LEN	2
#define MODBUS_RECV_CNT	3
#define MODBUS_RECV_SYSID 4
#define MODBUS_RECV_TYPE 5
#define MODBUS_RECV_DATA 6
#define MODBUS_RECV_CHECKSUM 7
#define MODBUS_RECV_CHECKSUM_CRC_HIGH 8
#define MODBUS_RECV_CHECKSUM_CRC_LOW 9

struct T_GLOBAL_BOOL_MODBUS global_bool_modbus;

static float modbus_send;
static float modbus_receive;
static float modbus_need_to_send;

unsigned short get_crc16(unsigned char *buf, unsigned short len);
unsigned short exchange_short_high_low(unsigned short a_short_num);

int modbus_uart_init()
{
	uart_device.uart_name=UART_MODBUS;

	uart_device.baudrate=UART_MODBUS_BAUD;
	uart_device.databits=UART_MODBUS_DATABITS;
	uart_device.parity=UART_MODBUS_PARITY;
	uart_device.stopbits=UART_MODBUS_STOPBITS;

	modbus_send=clock_gettime_s();

	uart_device.uart_num=open_uart_dev(uart_device.uart_name);

    uart_device.ptr_fun=read_modbus_data;

    set_uart_opt( uart_device.uart_name, \
                  uart_device.baudrate,\
                  uart_device.databits,\
                  uart_device.parity,\
                  uart_device.stopbits);

    create_uart_pthread(&uart_device);

	return 0;
}

int read_modbus_data(unsigned char *buf, unsigned int len)
{
	unsigned char check_crc_buf[256];

	static unsigned char _buffer[MODBUS_RECV_BUF_LEN];
	static unsigned char _pack_recv_buf[MODBUS_RECV_BUF_LEN];
	static int _pack_buf_len = 0;

	static int modbus_recv_state = 0;

	static int _pack_recv_device_id=0;
	static int _pack_recv_func_code=0;
	static int _pack_recv_len = 0;
	static int _check_crc_high=0;
	static int _check_crc_low=0;

	int _length;
	int i = 0;
	unsigned char c;
	unsigned short crc16=0;

	memcpy(_buffer, buf, len);
#if 0
	for(i=0;i<len;i++)
	{
		printf("read modbus data is %x\n",_buffer[i]);
	}
#endif

	_length = len;
	if(_buffer[_length-1]==0 && _buffer[_length-2]==0)
		_length=len-2;

	for (i = 0; i<_length; i++)
	{
		c = _buffer[i];
		switch (modbus_recv_state)
		{
		case MODBUS_RECV_DEVICE_ID:
			/*
			 * device id
			 */
			if ( 0x01== c || 0x02==c || 0x04==c || 0x08==c || 0x03==c)
			{
				_pack_recv_device_id=c;
				modbus_recv_state = MODBUS_RECV_FUNC_CODE;
			}
			break;
		case MODBUS_RECV_FUNC_CODE:
			/*
			 * function code
			 */
			if ( 0x01==c || 0x03==c || 0x05==c || 0x06==c || 0x0F==c)
			{
				_pack_recv_func_code=c;
				modbus_recv_state = MODBUS_RECV_LEN;

				/*
				 * 如果要求高的话，按理说应该设备的地址id要一致，功能号也需要一致才可以确认是收到了数据，
				 * 但是这里暂时只确认发送的和收到的id地址一致就确认接收完全
				 */
				//if(_pack_recv_device_id==global_bool_modbus.modbus_is_sending_id && _pack_recv_device_id==global_bool_modbus.modbus_is_sending_func_code)
				if((_pack_recv_device_id==global_bool_modbus.modbus_is_sending_id))
				{
#if 0
					global_bool_modbus.modbus_is_sending=FALSE;
					printf("收到了之前请求的数据，准备下一个modbus 485数据的发送\n");

					modbus_receive=clock_gettime_s();
					global_bool_modbus.modbus_send_and_receive_time=modbus_receive-modbus_send;
					//printf("485总线从发送到收到数据间隔 秒:%f\n",global_bool_modbus.modbus_send_and_receive_time);
					fflush(stdout);
#endif
				}
			}
			else
			{
				modbus_recv_state = MODBUS_RECV_DEVICE_ID;
			}
			break;
		case MODBUS_RECV_LEN:
			_pack_recv_len = c;
			modbus_recv_state = MODBUS_RECV_DATA;

			_pack_buf_len = 0;
			break;
		case MODBUS_RECV_DATA:
			_pack_recv_buf[_pack_buf_len] = c;

			_pack_buf_len++;
			if (_pack_buf_len >= _pack_recv_len)
			{
				modbus_recv_state = MODBUS_RECV_CHECKSUM_CRC_HIGH;
			}
			break;
		case MODBUS_RECV_CHECKSUM_CRC_HIGH:
			_check_crc_high = c;
			modbus_recv_state = MODBUS_RECV_CHECKSUM_CRC_LOW;
			break;
		case MODBUS_RECV_CHECKSUM_CRC_LOW:
			_check_crc_low = c;
			check_crc_buf[0]=_pack_recv_device_id;
			check_crc_buf[1]=_pack_recv_func_code;
			check_crc_buf[2]=_pack_recv_len;

			memcpy(&check_crc_buf[3], _pack_recv_buf ,_pack_recv_len);

			crc16=get_crc16(check_crc_buf,3 + _pack_recv_len);
			if(_check_crc_high * 256 +_check_crc_low == crc16)
			{
				switch(_pack_recv_device_id)
				{
				case POWER_ID:
					/*
					 * 充电机
					 */
					read_power_data(&read_power, _pack_recv_buf, _pack_recv_len);
					modbus_recv_state = 0;
					break;
				case TOGGLE_DISCHARGE_ID:
					/*
					 * 放电切换器
					 */
					read_toggle_data(&read_toggle, _pack_recv_buf, _pack_recv_len);
					modbus_recv_state = 0;
					break;
				default:
					break;
				}
			}
			else
			{
				modbus_recv_state = 0;
			}
		}
	}

	return 0;
}

int send_modbus_data(unsigned char dev_id, unsigned char function_code, unsigned char *buf, unsigned int len)
{
	unsigned char send_buf[256];
	unsigned short crc_16=0;

	int i;
    float delt_send_time_float;
#if 0
    unsigned char send_buf_plus[256];
    send_buf_plus[0]=0;
    send_buf_plus[1]=0;
    send_buf_plus[2]=0;
    send_buf_plus[3]=0;
#endif
	send_buf[0]=dev_id;//1
	send_buf[1]=function_code;//2

	memcpy(&send_buf[2],buf,len);//数据

	crc_16=get_crc16(send_buf,len+2);//3 & 4
	//crc_16=exchange_short_high_low(crc_16);
	send_buf[len+2] =(unsigned char)(crc_16 >> 8 & 0xFF);/*高位在前，先发送*/
	send_buf[len+3] = (unsigned char)(crc_16 & 0x00FF);
#if 1
	for(i=0;i<len+4;i++)
	{
		printf("send modbus data is %x\n",send_buf[i]);
	}
#endif

	modbus_need_to_send=clock_gettime_s();
	delt_send_time_float=modbus_need_to_send-modbus_send;
	//printf("delt_send_time_float=%f\n",delt_send_time_float);
	fflush(stdout);
#if 0
	if(!global_bool_modbus.modbus_is_sending)
	{
		global_bool_modbus.modbus_is_sending=TRUE;
		modbus_send=clock_gettime_s();

		global_bool_modbus.modbus_is_sending_id=send_buf[0];
		global_bool_modbus.modbus_is_sending_func_code=send_buf[1];
		send_uart_data(UART_MODBUS, (char *)send_buf, len+4);
	}
	else if( (delt_send_time_float > MODBUS_MAX_WAIT_TIME ))
	{
		global_bool_modbus.modbus_is_sending=FALSE;
		delt_send_time_float=0;
		modbus_need_to_send=0.0;
		printf("正在发送的id是=%d\n",global_bool_modbus.modbus_is_sending_id);
		printf("modbus总线等待超时，强制发送\n");
		fflush(stdout);
	}
#else
	//这种情况就是不考虑485是否正在发送数据，直接发送
	send_uart_data(UART_MODBUS, (char *)send_buf, len+4);
#endif


	//sleep_ms(300);
	sleep_ms(500);
	//sleep_ms(200);
	//sleep_ms(150);
	//sleep_ms(100);

	return 0;
}


int modbus_uart_close(unsigned int uart_num)
{
    uart_device.uart_name=UART_MODBUS;
    close_uart_dev(uart_device.uart_name);

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

unsigned short exchange_short_high_low(unsigned short a_short_num)
{
	unsigned char high = 0;
	//unsigned char low = 0;
	unsigned short return_exchange_short = 0;


	high =(unsigned char)(a_short_num >> 8 & 0xFF);
	//low = (unsigned char)(a_short_num & 0x00FF);

	return_exchange_short = a_short_num << 8 & 0xFF00;
	return_exchange_short = return_exchange_short | ((unsigned short)(high));

	return return_exchange_short;

}

