/*
 * rocket.c
 *
 *  Created on: Oct 12, 2016
 *      Author: wangbo
 */

#include <stdio.h>
#include <string.h>

/*转换int或者short的字节顺序，该程序arm平台为大端模式，地面站x86架构为小端模式*/
#include <byteswap.h>

/*open() close()所需头文件*/
#include <unistd.h>

#include "uart.h"
#include "utilityfunctions.h"

#include "rocket.h"
#include "global.h"

#define ROCKET_RECV_BUF_LEN 512

#define ROCKET_RECV_HEAD1    0
#define ROCKET_RECV_HEAD2    1
#define ROCKET_RECV_HEAD3    2
#define ROCKET_RECV_HEAD4    3

#define ROCKET_RECV_LEN1     4
#define ROCKET_RECV_LEN2    5
#define ROCKET_RECV_DATA     6
#define ROCKET_RECV_CHECKSUM 7

#define TYPE_COMMAND 1
#define TYPE_AIR_SOUNDING 2//探空数据

T_ROCKET read_rocket;
T_ROCKET write_rocket;
T_AIR_SOUNDING air_sounding_rocket;

static int rocket_recv_state = 0;
static int rocket_uart_init_real(struct T_UART_DEVICE_PROPERTY *ptr_uart_device_property);


int rocket_uart_init(unsigned int uart_num)
{
	uart_device_property.uart_num=uart_num;
	uart_device_property.baudrate=UART_ROCKET_BAUD;
	uart_device_property.databits=UART_ROCKET_DATABITS;
	uart_device_property.parity=UART_ROCKET_PARITY;
	uart_device_property.stopbits=UART_ROCKET_STOPBITS;

	rocket_uart_init_real(&uart_device_property);

	return 0;
}

int rocket_uart_init_real(struct T_UART_DEVICE_PROPERTY *ptr_uart_device_property)
{

	if(-1!=(uart_fd[ptr_uart_device_property->uart_num]=open_uart_dev(uart_fd[ptr_uart_device_property->uart_num], ptr_uart_device_property->uart_num)))
	{
		printf("uart_fd[UART_ROCKET]=%d\n",uart_fd[ptr_uart_device_property->uart_num]);
		set_uart_opt( uart_fd[ptr_uart_device_property->uart_num], \
					  ptr_uart_device_property->baudrate,\
					  ptr_uart_device_property->databits,\
					  ptr_uart_device_property->parity,\
					  ptr_uart_device_property->stopbits);

		uart_device_pthread(ptr_uart_device_property->uart_num);
	}

	return 0;
}

int read_rocket_data(T_ROCKET *ptr_read_rocket,unsigned char *buf, unsigned int len)
{
	static unsigned char _buffer[ROCKET_RECV_BUF_LEN];

	static unsigned char _pack_recv_type;
	//static int _pack_recv_cnt = 0;
	static int _pack_recv_len = 0;
	static unsigned char _pack_recv_buf[ROCKET_RECV_BUF_LEN];
	static int _pack_buf_len = 0;
	static unsigned char _checksum = 0;

	int _length;
	int i = 0;
	unsigned char c;
	//unsigned char _sysid;

	memcpy(_buffer, buf, len);

	/*显示收到的数据*/
#if 0
	printf("rocket data buf=\n");
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
		switch (rocket_recv_state)
		{
		case ROCKET_RECV_HEAD1:
			if (c == 0xEB)
			{
				rocket_recv_state = ROCKET_RECV_HEAD2;
			}
			break;
		case ROCKET_RECV_HEAD2:
			if (c == 0xEB)
			{
				rocket_recv_state = ROCKET_RECV_HEAD3;
			}
			else
			{
				rocket_recv_state = ROCKET_RECV_HEAD1;
			}
			break;
		case ROCKET_RECV_HEAD3:
			if (c == 0xEB)
			{
				rocket_recv_state = ROCKET_RECV_HEAD4;
			}
			else
			{
				rocket_recv_state = ROCKET_RECV_HEAD1;
			}
			_pack_buf_len = 0;
			break;
		case ROCKET_RECV_HEAD4:
			if (c == 0xEB)
			{
				rocket_recv_state = ROCKET_RECV_LEN1;
			}
			else
			{
				rocket_recv_state = ROCKET_RECV_HEAD1;
			}
			_pack_buf_len = 0;
			break;
		case ROCKET_RECV_LEN1:
			//_pack_recv_len = c;
			_pack_recv_len = c -10 -1;//10=4+2+2帧头+长度+帧尾 1是校验位 剩下的是数据位长度
			rocket_recv_state = ROCKET_RECV_LEN2;
			if(_pack_recv_len==0x0E)
			{
				_pack_recv_type=TYPE_COMMAND;
			}
			else if(_pack_recv_len==0x22)
			{
				_pack_recv_type=TYPE_AIR_SOUNDING;//探空数据
			}
			_pack_buf_len = 0;
			break;
		case ROCKET_RECV_LEN2:
			rocket_recv_state = ROCKET_RECV_DATA;
			_pack_buf_len = 0;
			_checksum=0;
			break;
		case ROCKET_RECV_DATA:
			_pack_recv_buf[_pack_buf_len] = c;
			_checksum += c;
			_pack_buf_len++;
			if (_pack_buf_len >= _pack_recv_len)
			{
				rocket_recv_state = ROCKET_RECV_CHECKSUM;
			}
			break;
		case ROCKET_RECV_CHECKSUM:
			if (_checksum == c)
			{
				switch (_pack_recv_type)
				{

				case TYPE_COMMAND:
					/*处理指令数据包*/
					printf("_pack_recv_len=%d\n",_pack_recv_len);
					printf("read_rocket占用%d个字节\n",sizeof(read_rocket));
					memcpy(&read_rocket, _pack_recv_buf, _pack_recv_len);
					global_bool.bool_get_rocket_command=TRUE;

					rocket_recv_state = 0;
					break;
				case TYPE_AIR_SOUNDING:
					/*处理探空数传数据包*/
					printf("_pack_recv_len=%d\n",_pack_recv_len);
					printf("air_sounding_rocket占用%d个字节\n",sizeof(air_sounding_rocket));
					memcpy(&air_sounding_rocket, _pack_recv_buf, _pack_recv_len);
					global_bool.bool_get_rocket_ari_sounding=TRUE;

					//如果字节顺序是相反的，则使用下面方法
					air_sounding_rocket.temp=__bswap_16(air_sounding_rocket.temp);
					air_sounding_rocket.longitude=__bswap_32(air_sounding_rocket.longitude);
					rocket_recv_state = 0;
					break;

				default:
					break;

				}
			}
			else
			{
				rocket_recv_state = 0;
			}
		}
	}

	return 0;
}

#define pilot2rocket_start_launch 0x01
#define pilot2rocket_has_stop_send_attitude 0x02
#define pilot2rocket_has_receive_fail_launch_no_window 0x03
#define pilot2rocket_has_receive_fail_launch_no_rocket 0x04
#define pilot2rocket_has_receive_success 0x05
#define pilot2rocket_has_finished_sounding 0x06
#define pilot2rocket_please_repeat 0x07

int write_rocket_data(T_ROCKET *ptr_write_rocket)
{
	unsigned char buf[14]={0};
	static int len=14;

	buf[0]=0xEB;
	buf[1]=0xEB;
	buf[2]=0xEB;
	buf[3]=0xEB;

	/*长度*/
	buf[4]=0x0E;
	buf[5]=0x00;

	/*数据*/
	//buf[6]=ptr_write_rocket->device_num;
	buf[6]=0x02;
	buf[7]=ptr_write_rocket->function_num;
	buf[8]=ptr_write_rocket->information;
	buf[9]=buf[6]+buf[7]+buf[8];

	buf[10]=0xEE;
	buf[11]=0xEE;
	buf[12]=0xEE;
	buf[13]=0xEE;

	send_uart_data(uart_fd[UART_ROCKET], (char *)buf, len);

	return 0;
}
int write_rocket_attitude()
{
	//write_rocket.function_num=
	return 0;
}
int start_launch_rocket()
{
	return 0;
}
int reply_stop_send_attitude()
{
	write_rocket.function_num=pilot2rocket_has_stop_send_attitude;
	write_rocket_data(&write_rocket);

	return 0;
}
int reply_no_window()
{
	write_rocket.function_num=pilot2rocket_has_receive_fail_launch_no_window;
	write_rocket_data(&write_rocket);

	return 0;
}
int reply_no_available_rocket()
{
	write_rocket.function_num=pilot2rocket_has_receive_fail_launch_no_rocket;
	write_rocket_data(&write_rocket);

	return 0;
}
int reply_received_launch_success_report()
{
	write_rocket.function_num=pilot2rocket_has_receive_success;
	write_rocket_data(&write_rocket);

	return 0;
}
int reply_received_finished_sounding()
{
	write_rocket.function_num=pilot2rocket_has_finished_sounding;
	write_rocket_data(&write_rocket);

	return 0;
}
int reply_please_repeat()
{
	write_rocket.function_num=pilot2rocket_please_repeat;
	write_rocket_data(&write_rocket);

	return 0;
}

#define rocket2pilot_is_passing_launch_request 0x01
#define rocket2pilot_request_send_attitude 0x02
#define rocket2pilot_request_stop_attitude 0x03
#define rocket2pilot_no_window_stop 0x04
#define rocket2pilot_useup_rocket_fail 0x05
#define rocket2pilot_success_launch_and_prepare_to_receive_air_sounding 0x06
#define rocket2pilot_has_down_into_sea 0x07
#define rocket2pilot_please_repeat 0x08


/*
 * 光是从rocket中read数据还是不够的，
 * 因为获取数据是原始数据，
 * 总是需要解析的，decode总是跟read相结合在一起
 */
int decode_rocket_command(T_ROCKET *ptr_read_rocket)
{
	switch(ptr_read_rocket->function_num)
	{
	case rocket2pilot_is_passing_launch_request:
		/*正在转达发射请求*/
		break;
	case rocket2pilot_request_send_attitude:
		/*请求发送姿态数据*/
		/*既然火箭请求姿态数据，那么现在就应该发送姿态数据给火箭*/
		global_bool.bool_rocket_request_attitude=TRUE;
		write_rocket_attitude();

		break;
	case rocket2pilot_request_stop_attitude:
		/*请求停止姿态数据*/
		/*这个时候应该就不再发送姿态数据*/
		global_bool.bool_rocket_request_attitude=FALSE;
		break;
	case rocket2pilot_no_window_stop:
		/*因为没有窗口，发射行动中止*/

		break;
	case rocket2pilot_useup_rocket_fail:
		/*火箭用完，发射失败*/
		break;
	case rocket2pilot_success_launch_and_prepare_to_receive_air_sounding:
		/*火箭x号，发射成功，请准备接收探空数据*/
		break;
	case rocket2pilot_has_down_into_sea:
		/*火箭x号，已经落海，传输结束*/
		break;
	case rocket2pilot_please_repeat:
		/*刚才收到的指令没有通过校验，请重发*/
		break;
	default:
		break;
	}

	return 0;
}

int rocket_uart_close(unsigned int uart_num)
{
	close(uart_fd[uart_num]);

	return 0;
}
