/*
 * rocket.c
 *
 *  Created on: Oct 12, 2016
 *      Author: wangbo
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*转换int或者short的字节顺序，该程序arm平台为大端模式，地面站x86架构为小端模式*/
#include <byteswap.h>

/*open() close()所需头文件*/
#include <unistd.h>

#include "uart.h"
#include "global.h"

#include "rocket.h"

#define ROCKET_RECV_BUF_LEN 512

#define ROCKET_RECV_HEAD1    0
#define ROCKET_RECV_HEAD2    1
#define ROCKET_RECV_HEAD3    2
#define ROCKET_RECV_HEAD4    3

#define ROCKET_RECV_LEN1     4
#define ROCKET_RECV_LEN2     5
#define ROCKET_RECV_DATA     6
#define ROCKET_RECV_CHECKSUM 7

/*2种协议的火箭数据包*/
#define TYPE_COMMAND 1
#define TYPE_AIR_SOUNDING 2//探空数据

T_ROCKET read_rocket;
T_ROCKET write_rocket;
T_ROCKET_AIR_SOUNDING rocket_air_sounding;
T_ROCKET_ATTITUDE rocket_attitude;

static int rocket_recv_state = 0;
static int decode_rocket_command(T_ROCKET *ptr_read_rocket);

int rocket_uart_init()
{
	uart_device.uart_name=UART_ROCKET;

    uart_device.baudrate=UART_ROCKET_BAUD;
    uart_device.databits=UART_ROCKET_DATABITS;
    uart_device.parity=UART_ROCKET_PARITY;
    uart_device.stopbits=UART_ROCKET_STOPBITS;

    uart_device.uart_num=open_uart_dev(uart_device.uart_name);

    uart_device.ptr_fun=read_rocket_data;

    set_uart_opt( uart_device.uart_name, \
                  uart_device.baudrate,\
                  uart_device.databits,\
                  uart_device.parity,\
                  uart_device.stopbits);

    create_uart_pthread(&uart_device);

	return 0;
}

#define position0  0
#define position1  2
#define position2  4
#define position3  6
#define position4  8
#define position5  10
#define position6  14
#define position7  18
#define position8  22
#define position9  23
int decode_rocket_air_sounding_data(T_ROCKET_AIR_SOUNDING *ptr_rocket_air_sounding,char *buf)
{
    unsigned char size_byte;
    unsigned char index_i;//指示数据的字节位置
    char buf_frame[125]={0};

    static int len=23;//火箭探空数据包，实际传递的信息，从温度开始到编号截至，总共23个字节

    //buf的第一个数据是温度，最后一个是编号，不包括校验
    memcpy(&buf_frame[0],buf,len);//需要len个字节

    /*显示收到的数据*/
#if 0
    int i=0;
    printf("aws data buf=\n");
    //for(i=0;i<len;i++)
    for(i=0;i<36;i++)
    {
        printf("%c",buf_frame[i]);
    }
    printf("\n");
#endif

    index_i=position0;
    size_byte=position1-position0;
    memcpy(&ptr_rocket_air_sounding->temp,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->temp=%d\n",ptr_rocket_air_sounding->temp);

    index_i=position1;
    size_byte=position2-position1;
    memcpy(&ptr_rocket_air_sounding->humidity,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->humidity=%d\n",ptr_rocket_air_sounding->humidity);

    index_i=position2;
    size_byte=position3-position2;
    memcpy(&ptr_rocket_air_sounding->air_pressure,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->air_pressure=%d\n",ptr_rocket_air_sounding->air_pressure);

    index_i=position3;
    size_byte=position4-position3;
    memcpy(&ptr_rocket_air_sounding->wind_speed,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->wind_speed=%d\n",ptr_rocket_air_sounding->wind_speed);

    index_i=position4;
    size_byte=position5-position4;
    memcpy(&ptr_rocket_air_sounding->wind_dir,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->wind_dir=%d\n",ptr_rocket_air_sounding->wind_dir);

    index_i=position5;
    size_byte=position6-position5;
    memcpy(&ptr_rocket_air_sounding->longitude,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->longitude=%d\n",ptr_rocket_air_sounding->longitude);

    index_i=position6;
    size_byte=position7-position6;
    memcpy(&ptr_rocket_air_sounding->latitude,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->latitude=%d\n",ptr_rocket_air_sounding->latitude);

    index_i=position7;
    size_byte=position8-position7;
    memcpy(&ptr_rocket_air_sounding->height,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->height=%d\n",ptr_rocket_air_sounding->height);

    index_i=position8;
    size_byte=position9-position8;
    memcpy(&ptr_rocket_air_sounding->number,buf_frame+index_i,size_byte);
    printf("ptr_rocket_air_sounding->number=%d\n",ptr_rocket_air_sounding->number);

    return 0;
}

int read_rocket_data(unsigned char *buf, unsigned int len)
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
		printf("%x,",buf[i]);
	}
	printf("\n");
#endif

	_length = len;

	for (i = 0; i<_length; i++)
	{
		c = _buffer[i];
		//printf("state=%d\n",rocket_recv_state);
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
			_pack_recv_len = c -10 -1;//10=4+2+4 即帧头4+长度2+帧尾4 1是校验位 剩下的是数据位长度
			rocket_recv_state = ROCKET_RECV_LEN2;
			if(c==0x0E)//如果收到的字符c表示的长度为14，则是命令包
			{
				_pack_recv_type=TYPE_COMMAND;
			}
			else if(c==0x22)
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
					printf("read_rocket占用%d个字节\n",sizeof(T_ROCKET));
					/*按照惯例这里应该是decode函数，但是因为read_rocket是4个单字节组成的数据包，用memcpy就可以准确解析*/
					memcpy(&read_rocket, _pack_recv_buf, _pack_recv_len);
					global_bool.bool_get_rocket_command=TRUE;
					//解析指令数据包
					decode_rocket_command(&read_rocket);

					rocket_recv_state = 0;
					break;
				case TYPE_AIR_SOUNDING:
					/*处理探空数传数据包*/
					printf("_pack_recv_len=%d\n",_pack_recv_len);
					printf("rocket_air_sounding占用%d个字节\n",sizeof(T_ROCKET_AIR_SOUNDING));
					global_bool.bool_get_rocket_air_sounding=TRUE;
					decode_rocket_air_sounding_data(&rocket_air_sounding,(char *)_pack_recv_buf);

#if 0
					memcpy(&rocket_air_sounding, _pack_recv_buf, _pack_recv_len);
					//设置了global_bool.bool_get_rocket_ari_sounding，接下来应该是解析数据包的
					//但是因为rocket_air_sounding是数据接收包，所以没有解析，直接赋值给了&rocket_air_sounding
					//但是游客能出现字节顺序不一致的情况，所以采用了下面的交换位置

					//如果字节顺序是相反的，则使用下面方法
					rocket_air_sounding.temp=__bswap_16(rocket_air_sounding.temp);
					rocket_air_sounding.humidity=__bswap_16(rocket_air_sounding.humidity);
					rocket_air_sounding.air_pressure=__bswap_16(rocket_air_sounding.air_pressure);
					rocket_air_sounding.wind_speed=__bswap_16(rocket_air_sounding.wind_speed);
					rocket_air_sounding.wind_dir=__bswap_16(rocket_air_sounding.wind_dir);

					rocket_air_sounding.longitude=__bswap_32(rocket_air_sounding.longitude);
					rocket_air_sounding.latitude=__bswap_32(rocket_air_sounding.latitude);
					rocket_air_sounding.height=__bswap_32(rocket_air_sounding.height);
#endif
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

#define pilot2rocket_start_charge_air_sounding 0x08
#define pilot2rocket_has_received_charge_start 0x09
#define pilot2rocket_has_received_charge_done 0x0A

int write_rocket_cmd(T_ROCKET *ptr_write_rocket)
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
	buf[6]=0x02;
	buf[7]=ptr_write_rocket->function_num;
	buf[8]=ptr_write_rocket->information;
	buf[9]=buf[6]+buf[7]+buf[8];

	buf[10]=0xEE;
	buf[11]=0xEE;
	buf[12]=0xEE;
	buf[13]=0xEE;

	buf[14]='\0';
	//printf("发送的=%s\n",buf);

    uart_device.uart_name=UART_ROCKET;
    send_uart_data(uart_device.uart_name, (char *)buf, len);
	printf("发送的=%s\n",buf);

	return 0;
}

int write_rocket_data(unsigned char *buf,unsigned int len)
{

    unsigned char send_buf[256]={0};
    int frame_len;//帧长度,参数len是数据长度
    unsigned short check=0;
    int i;

    for(i=0;i<len;i++)
    {
        check+=buf[i];
    }

    frame_len=len+4+2+2+4;//数据长度+帧头4+长度2+校验2+4帧尾
    //printf("rocket frame len=%d\n",frame_len);

    send_buf[0]=0xEB;
    send_buf[1]=0xEB;
    send_buf[2]=0xEB;
    send_buf[3]=0xEB;

    send_buf[4]=frame_len;
    send_buf[5]=0x00;

    memcpy(&send_buf[6],buf,len);

    //校验字节
    memcpy(&send_buf[len+6],&check,2);

    //帧尾
    send_buf[len+8]=0xEE;
    send_buf[len+9]=0xEE;
    send_buf[len+10]=0xEE;
    send_buf[len+11]=0xEE;

    //send_uart_data(uart_fd[UART_ROCKET], (char *)send_buf, frame_len);
    uart_device.uart_name=UART_ROCKET;
    send_uart_data(uart_device.uart_name, (char *)buf, frame_len);//注意这里是frame_len也就是真正发送的字节长度
#if 0
    for(i=0;i<frame_len;i++)
    {
        printf("send_buf=%x\n",send_buf[i]);
    }
#endif

    return 0;
}

int write_rocket_attitude()
{
    unsigned char send_buf[256]={0};

    rocket_attitude.longitude=116123456;//116.123456 10e-6
    rocket_attitude.latitude=39123456;//39.123456 10e-6

    rocket_attitude.height=111;//11.1 10e-1
    rocket_attitude.pitch=100;//1度 10e-2
    rocket_attitude.roll=100;//1度 10e-2
    rocket_attitude.yaw=100;//1度 10e-2

    memcpy(send_buf,&rocket_attitude,sizeof(rocket_attitude));

    write_rocket_data(send_buf,sizeof(rocket_attitude));

	return 0;
}

int start_launch_rocket()
{
    write_rocket.function_num=pilot2rocket_start_launch;
    write_rocket.information=0x00;
    write_rocket_cmd(&write_rocket);

	return 0;
}

int reply_stop_send_attitude()
{
	write_rocket.function_num=pilot2rocket_has_stop_send_attitude;
	write_rocket.information=0x00;
	write_rocket_cmd(&write_rocket);

	return 0;
}
int reply_no_window()
{
	write_rocket.function_num=pilot2rocket_has_receive_fail_launch_no_window;
	write_rocket.information=0x00;
	write_rocket_cmd(&write_rocket);

	return 0;
}
int reply_fail_no_available_rocket()
{
	write_rocket.function_num=pilot2rocket_has_receive_fail_launch_no_rocket;
	write_rocket.information=0x01;
	write_rocket_cmd(&write_rocket);

	return 0;
}
int reply_fail_device_error()
{
    write_rocket.function_num=pilot2rocket_has_receive_fail_launch_no_rocket;
    write_rocket.information=0x02;
    write_rocket_cmd(&write_rocket);

    return 0;
}

int reply_received_launch_success1_report()
{
	write_rocket.function_num=pilot2rocket_has_receive_success;
	write_rocket.information=0x01;
	write_rocket_cmd(&write_rocket);

	return 0;
}
int reply_received_launch_success2_report()
{
    write_rocket.function_num=pilot2rocket_has_receive_success;
    write_rocket.information=0x02;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_received_launch_success3_report()
{
    write_rocket.function_num=pilot2rocket_has_receive_success;
    write_rocket.information=0x03;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_received_launch_success4_report()
{
    write_rocket.function_num=pilot2rocket_has_receive_success;
    write_rocket.information=0x04;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_received_finished_sounding1()
{
	write_rocket.function_num=pilot2rocket_has_finished_sounding;
	write_rocket.information=0x01;
	write_rocket_cmd(&write_rocket);

	return 0;
}
int reply_received_finished_sounding2()
{
    write_rocket.function_num=pilot2rocket_has_finished_sounding;
    write_rocket.information=0x02;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_received_finished_sounding3()
{
    write_rocket.function_num=pilot2rocket_has_finished_sounding;
    write_rocket.information=0x03;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_received_finished_sounding4()
{
    write_rocket.function_num=pilot2rocket_has_finished_sounding;
    write_rocket.information=0x04;
    write_rocket_cmd(&write_rocket);

    return 0;
}
int reply_please_repeat()
{
	write_rocket.function_num=pilot2rocket_please_repeat;
	write_rocket_cmd(&write_rocket);

	return 0;
}

int start_charege_air_sounding()
{
    write_rocket.function_num=pilot2rocket_start_charge_air_sounding;
    write_rocket_cmd(&write_rocket);

    return 0;
}

int reply_has_received_number2_start()
{
    write_rocket.function_num=pilot2rocket_has_received_charge_start;
    write_rocket.information=0x02;
    write_rocket_cmd(&write_rocket);
    return 0;
}

int reply_has_received_number3_start()
{
    write_rocket.function_num=pilot2rocket_has_received_charge_start;
    write_rocket.information=0x03;
    write_rocket_cmd(&write_rocket);
    return 0;
}

int reply_has_received_charge_done()
{
    write_rocket.function_num=pilot2rocket_has_received_charge_done;
    write_rocket.information=0x00;
    write_rocket_cmd(&write_rocket);

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
	    printf("rocket-->pilot:正在转达发射请求\n");
		break;
	case rocket2pilot_request_send_attitude:
		/*请求发送姿态数据*/
		/*既然火箭请求姿态数据，那么现在就应该发送姿态数据给火箭*/
	    printf("rocket-->pilot:请求发送姿态数据\n");
		global_bool.bool_rocket_request_attitude=TRUE;
		write_rocket_attitude();
		break;
	case rocket2pilot_request_stop_attitude:
		/*请求停止姿态数据*/
		/*这个时候应该就不再发送姿态数据*/
	    printf("rocket-->pilot:请求停止姿态数据\n");
		global_bool.bool_rocket_request_attitude=FALSE;
		reply_stop_send_attitude();
		break;
	case rocket2pilot_no_window_stop:
		/*因为没有窗口，发射行动中止*/
	    printf("rocket-->pilot:因为没有窗口，发射行动中止\n");
		break;
	case rocket2pilot_useup_rocket_fail:
		/*火箭用完，发射失败*/
	    printf("rocket-->pilot:火箭用完，发射失败\n");
		break;
	case rocket2pilot_success_launch_and_prepare_to_receive_air_sounding:
		/*火箭x号，发射成功，请准备接收探空数据*/
	    printf("rocket-->pilot:火箭x号，发射成功，请准备接收探空数据\n");
		break;
	case rocket2pilot_has_down_into_sea:
		/*火箭x号，已经落海，传输结束*/
	    printf("rocket-->pilot:火箭x号，已经落海，传输结束\n");
		break;
	case rocket2pilot_please_repeat:
		/*刚才收到的指令没有通过校验，请重发*/
	    printf("rocket-->pilot:刚才收到的指令没有通过校验，请重发\n");
		break;
	default:
		break;
	}

	return 0;
}

int rocket_uart_close()
{
    uart_device.uart_name=UART_ROCKET;
    close_uart_dev(uart_device.uart_name);

	return 0;
}
