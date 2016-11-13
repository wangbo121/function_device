/*
 * modbus_toggle_discharge.c
 *
 *  Created on: Jul 23, 2016
 *      Author: wangbo
 */


#include <stdio.h>

#include "modbus_toggle_discharge.h"
#include "modbus_485.h"
#include "utilityfunctions.h"

struct T_TOGGLE query_toggle;
struct T_TOGGLE set_toggle;
struct T_TOGGLE read_toggle;

int write_query_toggle_discharge(struct T_TOGGLE *ptr_query_toggle, unsigned char data_type);
int write_set_toggle_discharge(struct T_TOGGLE *ptr_set_toggle, unsigned char data_type);
int read_toggle_data(struct T_TOGGLE *ptr_read_toggle, unsigned char buf[], unsigned int len);

static int toggle_discharge_battery_state;
#define DISCHARGE_BATTERY0 0
#define DISCHARGE_BATTERY1 1

int set_init_toggle_discharge()
{
    /*
     * 初始化切换器的最低电压110伏特，放电电压120
     */
#if 0
    //先把切换器去掉
    set_toggle.battery_low_limit=110;
    write_set_toggle_discharge(&set_toggle,20 );
    set_toggle.battery_high_limit=120;
    write_set_toggle_discharge(&set_toggle,21 );
#endif

    return 0;
}

//static float toggle_time=0.0;
int toggle_discharge_loop(struct T_TOGGLE *ptr_query_toggle,\
		                  struct T_TOGGLE *ptr_set_toggle, \
						  struct T_TOGGLE *ptr_read_toggle)
{
	/*查询电压，暂时不需要*/
	write_query_toggle_discharge(ptr_query_toggle,TOGGLE_BATTERY_VOLTAGE0);
	/*查询切换器的状态，查询第0个电池是否正在放电，请求放电，停止放电*/
	//write_query_toggle_discharge(ptr_query_toggle,TOGGLE_BATTERY_STATE0);

	switch(ptr_read_toggle->battery0_state)
	{
	case 0x0055:
		//printf("切换器 电池0 正在放电\n");
		global_bool_modbus.toggle_battery0_is_discharging=TRUE;
		global_bool_modbus.toggle_battery0_request_discharge=FALSE;
		global_bool_modbus.toggle_battery0_stop_discharge=FALSE;

		/*正在放电就不再请求充电*/
		global_bool_modbus.toggle_battery0_request_charge=FALSE;
		break;
	case 0x005A:
		//printf("切换器 电池0 请求放电\n");
		global_bool_modbus.toggle_battery0_is_discharging=FALSE;
		global_bool_modbus.toggle_battery0_request_discharge=TRUE;
		global_bool_modbus.toggle_battery0_stop_discharge=FALSE;

		/*
		 * 请求放电，还是需要进行再一次充电，因为请求充电是切换器的状态，只能一次次读取，切换器检测
		 * 电压到130伏时，瞬间就发出状态请求放电，但是充电循环由于执行速度慢还没有检测到充电完成，所以
		 * 需要继续请求充电，然后检测到充电完成，停止充电
		 * 不再请求充电
		 */
		break;
	case 0x00AA:
		//printf("切换器 电池0 停止放电\n");
		global_bool_modbus.toggle_battery0_is_discharging=FALSE;
		global_bool_modbus.toggle_battery0_request_discharge=FALSE;
		global_bool_modbus.toggle_battery0_stop_discharge=TRUE;

		global_bool_modbus.toggle_battery0_request_charge=TRUE;/*只有低于100伏时才会停止充电，意思也就是该充电了*/
		//global_bool_modbus.toggle_battery0_is_charging=TRUE;
		break;
	default:
		break;
	}

	write_query_toggle_discharge(ptr_query_toggle,TOGGLE_BATTERY_VOLTAGE1);
	/*查询切换器的状态，查询第1个电池是否正在放电，请求放电，停止放电*/
	//write_query_toggle_discharge(ptr_query_toggle,TOGGLE_BATTERY_STATE1);

	switch(ptr_read_toggle->battery1_state)
	{
	case 0x0055:
		//printf("切换器 电池1 正在放电\n");
		/*正在放电的话，我们就开始查询第2个电池状态*/
		global_bool_modbus.toggle_battery1_is_discharging=TRUE;
		global_bool_modbus.toggle_battery1_request_discharge=FALSE;
		global_bool_modbus.toggle_battery1_stop_discharge=FALSE;

		/*正在放电就不再请求充电*/
		global_bool_modbus.toggle_battery1_request_charge=FALSE;
		break;
	case 0x005A:
		//printf("切换器 电池1 请求放电\n");
		global_bool_modbus.toggle_battery1_is_discharging=FALSE;
		global_bool_modbus.toggle_battery1_request_discharge=TRUE;
		global_bool_modbus.toggle_battery1_stop_discharge=FALSE;
		/*
		 * 请求放电，还是需要进行再一次充电，因为请求充电是切换器的状态，只能一次次读取，切换器检测
		 * 电压到130伏时，瞬间就发出状态请求放电，但是充电循环由于执行速度慢还没有检测到充电完成，所以
		 * 需要继续请求充电，然后检测到充电完成，停止充电
		 * 不再请求充电
		 */
		break;
	case 0x00AA:
		//printf("切换器 电池1 停止放电\n");
		global_bool_modbus.toggle_battery1_is_discharging=FALSE;
		global_bool_modbus.toggle_battery1_request_discharge=FALSE;
		global_bool_modbus.toggle_battery1_stop_discharge=TRUE;/*只有低于100伏时才会停止充电，意思也就是该充电了*/

		global_bool_modbus.toggle_battery1_request_charge=TRUE;/*同时，请求充电机，给充电*/
		break;
	default:
		break;
	}
	//toggle_time=clock_gettime_s();
	//printf("*******************************************************************\n");
	//printf("*********time is :%f\n",toggle_time);
	//printf("每次查询切换器状态 切换器电池 0 的状态=%x\n",ptr_read_toggle->battery0_state);
	//printf("每次查询切换器状态 切换器电池 1 的状态=%x\n",ptr_read_toggle->battery1_state);

	//toggle_discharge_battery_state=DISCHARGE_BATTERY1;
	switch(toggle_discharge_battery_state)
	{
	case DISCHARGE_BATTERY0:
#if 0
		if(global_bool_modbus.toggle_battery0_request_discharge)
		{
			if(global_bool_modbus.toggle_battery0_is_charging)
			{
				//第0路电池，正在充电，不允许切换
				ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0xFF00) | 0x0055;
				write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

				/*
				 * 这里虽然不允许，切换到0通道放电，但是根据条件判断，通道0既然请求放电说明达到了放电电压
				 * 只是因为充电有滞后性，所以还不能切到0，因此我们就等待充电结束，
				 * 所以toggle_discharge_battery_state这个状态并不改变，继续等待通道0充满电
				 */
			}
			else
			{
				//第0路电池，没有充电，允许切换
				ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0xFF00) | 0x00AA;
				write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

				if(global_bool_modbus.toggle_battery1_is_discharging)
				{
					//printf("********************************\n");
					//printf("切换器:第1路电池正在放电 无法切换到放电通道 0\n");
				}
				else
				{
					set_toggle.discharge_num=0x0001;
					write_set_toggle_discharge(ptr_set_toggle, TOGGLE_DISCHARGE_NUM);
					global_bool_modbus.toggle_battery0_is_discharging=TRUE;

					/*放电的时候也不允许切换*/
					//第0路电池，正在放电，不允许切换
					ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0xFF00) | 0x0055;
					write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

					toggle_discharge_battery_state=DISCHARGE_BATTERY1;
				}

			}
		}
		else if(global_bool_modbus.toggle_battery0_stop_discharge)
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY1;
		}
		else if(global_bool_modbus.toggle_battery0_is_discharging)
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY1;
		}
		else
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY0;
		}
#else
		if(!global_bool_modbus.toggle_battery1_is_discharging)
		{
		    if(ptr_read_toggle->battery0_voltage>120)
		    //if(ptr_read_toggle->battery0_voltage>3000)
		    {
		        //printf("ptr_read_toggle->battery0_voltage>3000\n");
		        ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0xFF00) | 0x00AA;
                write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

                if(global_bool_modbus.toggle_battery0_is_discharging)
                {
                    //如果正在放电，则不再发送命令
                }
                else
                {
                    //如果没有在放电，就切换到1
                    printf("如果没有在放电，就切换到1\n");
                    printf("*********************************************\n");
                    printf("*********************************************\n");
                    printf("*********************************************\n");

                    set_toggle.discharge_num=0x0001;
                    //set_toggle.discharge_num=0x0002;
                    write_set_toggle_discharge(ptr_set_toggle, TOGGLE_DISCHARGE_NUM);
                    global_bool_modbus.toggle_battery0_is_discharging=TRUE;
                }

		    }
		    else
		    {
		        global_bool_modbus.toggle_battery0_is_discharging=FALSE;
		        toggle_discharge_battery_state=DISCHARGE_BATTERY1;
		    }
		}
		else
		{
		    toggle_discharge_battery_state=DISCHARGE_BATTERY1;
		}

#endif
		break;
	case DISCHARGE_BATTERY1:
#if 0
		/*电池1处于请求放电状态*/
		if(global_bool_modbus.toggle_battery1_request_discharge)
		{
			if(global_bool_modbus.toggle_battery1_is_charging)
			{
				//第1路电池，正在充电，不允许切换
				ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0x00FF) | 0x5500;
				write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);
			}
			else
			{
				//第1路电池，没有充电，允许切换
				ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0x00FF) | 0xAA00;
				write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

				if(global_bool_modbus.toggle_battery0_is_discharging)
				{
					//printf("********************************\n");
					//printf("切换器:第0路电池正在放电 无法切换到放电通道 1\n");
				}
				else
				{
					set_toggle.discharge_num=0x0002;
					write_set_toggle_discharge(ptr_set_toggle, TOGGLE_DISCHARGE_NUM);
					global_bool_modbus.toggle_battery1_is_discharging=TRUE;

					/*充电的时候不能切换，放电的时候也不能切换*/
					//第1路电池，正在放电，不允许切换
					ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0x00FF) | 0x5500;
					write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

					toggle_discharge_battery_state=DISCHARGE_BATTERY0;
				}
			}
		}
		else if(global_bool_modbus.toggle_battery1_stop_discharge)
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY0;
		}
		else if(global_bool_modbus.toggle_battery1_is_discharging)
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY0;
		}
		else
		{
			toggle_discharge_battery_state=DISCHARGE_BATTERY1;
		}
#else
		if(!global_bool_modbus.toggle_battery0_is_discharging)
        {
            if(ptr_read_toggle->battery1_voltage>120)
		    //if(1)
            {
                printf("切换器电池2电压=%u\n",ptr_read_toggle->battery1_voltage);
                ptr_set_toggle->charging_state=(ptr_set_toggle->charging_state & 0x00FF) | 0xAA00;
                write_set_toggle_discharge(ptr_set_toggle, TOGGLE_CHARGING_STATE);

                if(global_bool_modbus.toggle_battery1_is_discharging)
                {
                    //如果正在放电，则不再发送命令
                    printf("切换器电池2正在放电，则不再发送命令\n");

                }
                else
                {
                    //如果没有在放电，就切换到2
                    printf("如果没有在放电，就切换到2\n");

                    set_toggle.discharge_num=0x0002;
                    write_set_toggle_discharge(ptr_set_toggle, TOGGLE_DISCHARGE_NUM);
                    global_bool_modbus.toggle_battery1_is_discharging=TRUE;
                }
            }
            else
            {
                global_bool_modbus.toggle_battery1_is_discharging=FALSE;
                toggle_discharge_battery_state=DISCHARGE_BATTERY0;
            }
        }
		else
		{
		    toggle_discharge_battery_state=DISCHARGE_BATTERY0;
		}

#endif
		break;

	default:
		break;
	}

	return 0;
}

int write_query_toggle_discharge(struct T_TOGGLE *ptr_query_toggle, unsigned char data_type)
{
	unsigned char query_buf[4]={0};

	switch (data_type)
	{
	case TOGGLE_BATTERY_VOLTAGE0:
		query_buf[1]=ADDRESS_QUERY_TOGGLE_BATTERY_VOL0;/*register address */
		global_bool_modbus.query_toggle_battery_vol0=TRUE;
		//printf("切换器，查询电池0的电压\n");
		break;

	case TOGGLE_BATTERY_STATE0:
		query_buf[1]=ADDRESS_QUERY_TOGGLE_BATTERY_STA0;/*register address */
		global_bool_modbus.query_toggle_battery_sta0=TRUE;
		//printf("切换器，查询电池0的状态\n");
		break;

	case  TOGGLE_BATTERY_VOLTAGE1:
		query_buf[1]=ADDRESS_QUERY_TOGGLE_BATTERY_VOL1;/*register address */
		global_bool_modbus.query_toggle_battery_vol1=TRUE;
		//printf("切换器，查询电池1的电压\n");
		break;

	case TOGGLE_BATTERY_STATE1:
		query_buf[1]=ADDRESS_QUERY_TOGGLE_BATTERY_STA1;/*register address */
		global_bool_modbus.query_toggle_battery_sta1=TRUE;
		//printf("切换器，查询电池1的状态\n");
		break;

	default:
		break;
	}
	query_buf[0]=0;/*高地址为0*/

	query_buf[2]=0;
	query_buf[3]=1;/*1 Bytes register num*/

	/*function code =0x03: read*/
	send_modbus_data(TOGGLE_DISCHARGE_ID, 0x03, query_buf, sizeof (query_buf));
	//send_modbus_charge_toggle_data(TOGGLE_DISCHARGE_ID, 0x03, query_buf, sizeof (query_buf));

	return 0;
}

int write_set_toggle_discharge(struct T_TOGGLE *ptr_set_toggle, unsigned char data_type)
{
	unsigned char set_buf[4]={0};
	unsigned short set_data=0;

	switch(data_type)
	{
	case TOGGLE_CHARGING_STATE:
		set_buf[1]=ADDRESS_SET_TOGGLE_CHARGING_STA;/*register address */
		set_data=ptr_set_toggle->charging_state;
		//printf("切换器 设置切换器充电状态\n");
		//fflush(stdout);
		break;
	case TOGGLE_BATTERY_LOW_LIMIT:
		set_buf[1]=ADDRESS_SET_TOGGLE_BAT_LOW_LIMIT;/*register address */
		set_data=ptr_set_toggle->battery_low_limit*10;
		//printf("切换器 设置切换器电压下限\n");
		//fflush(stdout);
		break;
	case  TOGGLE_BATTERY_HIGH_LIMIT:
		set_buf[1]=ADDRESS_SET_TOGGLE_BAT_HIGH_LIMIT;/*register address */
		set_data=ptr_set_toggle->battery_high_limit*10;
		//printf("切换器 设置切换器电压上限\n");
		//fflush(stdout);
		break;
	case TOGGLE_DISCHARGE_NUM:
		set_buf[1]=ADDRESS_SET_TOGGLE_DISCHARGE_NUM;/*register address */
		set_data=ptr_set_toggle->discharge_num;
		//printf("切换器 设置切换器放电通道=%04x\n",ptr_set_toggle->discharge_num);
		//fflush(stdout);
		break;

	default:
		break;
	}

	set_buf[0]=0;/*高地址为0*/

	set_buf[2]=(unsigned char)(set_data >> 8 & 0x00FF);
	set_buf[3]=(unsigned char)(set_data & 0x00FF);

	/*function code =0x06: write*/
	send_modbus_data(TOGGLE_DISCHARGE_ID, 0x06, set_buf, sizeof (set_buf));
	//send_modbus_charge_toggle_data(TOGGLE_DISCHARGE_ID, 0x06, set_buf, sizeof (set_buf));

	return 0;
}

int read_toggle_data(struct T_TOGGLE *ptr_read_toggle, unsigned char buf[], unsigned int len)
{
	unsigned char high=0;
	unsigned char low=0;
	unsigned short data=0;

	high=buf[0];
	low=buf[1];
	//high=buf[1];
	//low=buf[0];

//	printf("buf[0]=%x\n",high);
	//printf("buf[1]=%x\n",low);
	data=(unsigned short)high << 8 | low;
	//printf("收到切换器数据\n");
	//printf("data=%d\n",data);

	if(global_bool_modbus.query_toggle_battery_vol0)
	{
		global_bool_modbus.query_toggle_battery_vol0=FALSE;

		//printf("切换器电池0电压=%d\n",data);
		ptr_read_toggle->battery0_voltage=(unsigned short)((float)data*0.1);
		//printf("ptr_read_toggle->battery0_voltage=%d\n",ptr_read_toggle->battery0_voltage);
		//printf("ptr_read_toggle->battery0_voltage=%x\n",ptr_read_toggle->battery0_voltage);
	}
	else if(global_bool_modbus.query_toggle_battery_sta0)
	{
		global_bool_modbus.query_toggle_battery_sta0=FALSE;

		ptr_read_toggle->battery0_state=data;
		//printf("切换器 电池0的状态=%x\n",ptr_read_toggle->battery0_state);
	}
	else if(global_bool_modbus.query_toggle_battery_vol1)
	{
		global_bool_modbus.query_toggle_battery_vol1=FALSE;

		//printf("切换器电池0电压=%d\n",data);
		ptr_read_toggle->battery1_voltage=(unsigned short)((float)data*0.1);
		//printf("ptr_read_toggle->battery1_voltage=%d\n",ptr_read_toggle->battery1_voltage);
		//printf("ptr_read_toggle->battery1_voltage=%x\n",ptr_read_toggle->battery1_voltage);
	}
	else if(global_bool_modbus.query_toggle_battery_sta1)
	{
		global_bool_modbus.query_toggle_battery_sta1=FALSE;

		ptr_read_toggle->battery1_state=data;
		//printf("切换器 电池1的状态=%x\n",ptr_read_toggle->battery1_state);
	}

	return 0;
}


