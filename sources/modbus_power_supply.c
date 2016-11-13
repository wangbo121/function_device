/*
 * modbus_power_supply.c
 *
 *  Created on: 2016年5月19日
 *      Author: wangbo
 */

#include <stdio.h>

#include "modbus_power_supply.h"
#include "modbus_485.h"

struct T_POWER query_power;
struct T_POWER set_power;
struct T_POWER read_power;

static unsigned short previous_write_type=0;
static unsigned short previous_write_set_power=0;


int set_init_power_supply()
{
    /*
     * 初始化充电机
     * 初始化状态要把充电机都关掉
     */
#if 0
    set_power.channel_num=1;
    write_set_power_supply(&set_power, CHANNEL_NUM);
    set_power.work_state=1;/*关机*/
    write_set_power_supply(&set_power, STATE);/*关机 关闭充电*/

    set_power.channel_num=2;
    write_set_power_supply(&set_power, CHANNEL_NUM);
    set_power.work_state=1;/*关机*/
    write_set_power_supply(&set_power, STATE);/*关机 关闭充电*/

    set_power.channel_num=3;
    write_set_power_supply(&set_power, CHANNEL_NUM);
    set_power.work_state=1;/*关机*/
    write_set_power_supply(&set_power, STATE);/*关机 关闭充电*/
#endif
    //设置充电机电流10A
    //设置充电机电流10A
    set_power.channel_num=2;
    write_set_power_supply(&set_power, CHANNEL_NUM);
    set_power.work_current=10;
    write_set_power_supply(&set_power, CURRENT);/*关机 关闭充电*/

    set_power.channel_num=1;
    write_set_power_supply(&set_power, CHANNEL_NUM);
    set_power.work_current=10;
    write_set_power_supply(&set_power, CURRENT);/*关机 关闭充电*/
    return 0;
}

int start_charge_power(struct T_POWER *ptr_query_power,
		               struct T_POWER *ptr_set_power,
					   struct T_POWER *ptr_read_power);

static int power_charge_battery_state=2;/*每次重启自驾仪的时候默认检查电池2的状态，否则会出错*/
#define CHARGE_BATTERY0 0
#define CHARGE_BATTERY1 1
#define CHARGE_BATTERY2 2
int power_charge_loop(struct T_POWER *ptr_query_power,\
		              struct T_POWER *ptr_set_power,\
					  struct T_POWER *ptr_read_power)
{
	unsigned char charge_done;

	switch(power_charge_battery_state)
	{
	case CHARGE_BATTERY0:
		ptr_set_power->battery_voltage=VOLTAGE_BATTERY0;
		ptr_set_power->channel_num=1;
		ptr_set_power->work_current=STOP_CURRENT_BATTERY0;

		if(global_bool_modbus.toggle_battery0_request_discharge)
		{
			ptr_set_power->channel_num=1;
			write_set_power_supply(&set_power, CHANNEL_NUM);
			ptr_set_power->work_state=1;/*关机*/
			write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

			/*
			 * 设置关机后，但是不一定已经关机了
			 * 所以需要查询一下是否真的关机，确实关机后，再返回
			 * global_bool_modbus.toggle_battery0_is_charging=FALSE;
			 */
			//start_charge_power(ptr_query_power,ptr_set_power, ptr_read_power);
			write_query_power_supply(ptr_query_power, STATE);
			if(ptr_read_power->power_state.is_off)
			{
				global_bool_modbus.toggle_battery0_request_charge=FALSE;
				global_bool_modbus.toggle_battery0_is_charging=FALSE;
				power_charge_battery_state=CHARGE_BATTERY1;
			}
			else
			{
				global_bool_modbus.toggle_battery0_is_charging=TRUE;
				power_charge_battery_state=CHARGE_BATTERY0;
			}
		}
		else if(global_bool_modbus.toggle_battery0_request_charge)
		{
			global_bool_modbus.toggle_battery0_is_charging=TRUE;

			charge_done=start_charge_power(ptr_query_power,ptr_set_power, ptr_read_power);
			write_query_power_supply(ptr_query_power, STATE);
			if(charge_done && ptr_read_power->power_state.is_off)
			{
				charge_done=0;

				global_bool_modbus.toggle_battery0_request_charge=FALSE;
				global_bool_modbus.toggle_battery0_is_charging=FALSE;
				power_charge_battery_state=CHARGE_BATTERY1;
			}
			else
			{
				global_bool_modbus.toggle_battery0_request_charge=TRUE;
				global_bool_modbus.toggle_battery0_is_charging=TRUE;
				power_charge_battery_state=CHARGE_BATTERY0;
			}
		}
		else if(global_bool_modbus.toggle_battery0_is_discharging)
		{

			ptr_set_power->channel_num=1;
			write_set_power_supply(&set_power, CHANNEL_NUM);
			ptr_set_power->work_state=1;/*关机*/
			write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

			global_bool_modbus.toggle_battery0_request_charge=FALSE;
			global_bool_modbus.toggle_battery0_is_charging=FALSE;

			power_charge_battery_state=CHARGE_BATTERY1;
		}
		else if(global_bool_modbus.toggle_battery0_stop_discharge)
		{
			power_charge_battery_state=CHARGE_BATTERY1;
		}
		else
		{
			power_charge_battery_state=CHARGE_BATTERY0;
		}
		break;
	case CHARGE_BATTERY1:
		ptr_set_power->battery_voltage=VOLTAGE_BATTERY1;
		ptr_set_power->channel_num=2;
		ptr_set_power->work_current=STOP_CURRENT_BATTERY1;

		if(global_bool_modbus.toggle_battery1_request_discharge)
		{
			set_power.channel_num=2;
			write_set_power_supply(&set_power, CHANNEL_NUM);
			ptr_set_power->work_state=1;/*关机*/
			write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

			/*
			 * 设置关机后，但是不一定已经关机了
			 * 所以需要查询一下是否真的关机，确实关机后，再返回
			 * global_bool_modbus.toggle_battery1_is_charging=FALSE;
			 */
			//start_charge_power(ptr_query_power,ptr_set_power, ptr_read_power);
			write_query_power_supply(ptr_query_power, STATE);
			if(ptr_read_power->power_state.is_off)
			{
				global_bool_modbus.toggle_battery1_request_charge=FALSE;
				global_bool_modbus.toggle_battery1_is_charging=FALSE;
				//power_charge_battery_state=CHARGE_BATTERY2;
				power_charge_battery_state=CHARGE_BATTERY0;
			}
			else
			{
				global_bool_modbus.toggle_battery1_is_charging=TRUE;
				power_charge_battery_state=CHARGE_BATTERY1;
			}
		}
		else if(global_bool_modbus.toggle_battery1_request_charge)
		{
			global_bool_modbus.toggle_battery1_is_charging=TRUE;

			charge_done=start_charge_power(ptr_query_power,ptr_set_power, ptr_read_power);
			write_query_power_supply(ptr_query_power, STATE);
			if(charge_done && ptr_read_power->power_state.is_off)
			{
				charge_done=0;

				global_bool_modbus.toggle_battery1_request_charge=FALSE;
				global_bool_modbus.toggle_battery1_is_charging=FALSE;
				//power_charge_battery_state=CHARGE_BATTERY2;
				power_charge_battery_state=CHARGE_BATTERY0;
			}
			else
			{
				global_bool_modbus.toggle_battery1_request_charge=TRUE;
				global_bool_modbus.toggle_battery1_is_charging=TRUE;
				power_charge_battery_state=CHARGE_BATTERY1;
			}
		}
		else if(global_bool_modbus.toggle_battery1_is_discharging)
		{
			set_power.channel_num=2;
			write_set_power_supply(&set_power, CHANNEL_NUM);
			ptr_set_power->work_state=1;/*关机*/
			write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

			global_bool_modbus.toggle_battery1_request_charge=FALSE;
			global_bool_modbus.toggle_battery1_is_charging=FALSE;

			//power_charge_battery_state=CHARGE_BATTERY2;
			power_charge_battery_state=CHARGE_BATTERY0;
		}
		else if(global_bool_modbus.toggle_battery1_stop_discharge)
		{
			//power_charge_battery_state=CHARGE_BATTERY2;
		    power_charge_battery_state=CHARGE_BATTERY0;
		}
		else
		{
			power_charge_battery_state=CHARGE_BATTERY1;
		}
		break;
	case CHARGE_BATTERY2:
		ptr_set_power->battery_voltage=VOLTAGE_BATTERY2;
		ptr_set_power->channel_num=3;

		charge_done=start_charge_power(ptr_query_power,ptr_set_power, ptr_read_power);
		if(charge_done)
		{
			charge_done=0;

			power_charge_battery_state=CHARGE_BATTERY0;
		}
		else
		{
			power_charge_battery_state=CHARGE_BATTERY2;
		}
		break;

	default:
		break;
	}

	return 0;
}

int write_query_power_supply(struct T_POWER *ptr_query_power, unsigned char data_type)
{
	unsigned char query_buf[4]={0};

	switch (data_type)
	{
	case VOLTAGE:
		query_buf[1]=ADDRESS_QUERY_POWER_VOL;/*register address */
		global_bool_modbus.query_power_voltage=TRUE;
		//printf("充电机 查询充电机电压\n");
		break;

	case CURRENT:
		query_buf[1]=ADDRESS_QUERY_POWER_CUR;/*register address */
		global_bool_modbus.query_power_current=TRUE;
		//printf("充电机 查询充电机电流\n");
		break;

	case  STATE:
		query_buf[1]=ADDRESS_QUERY_POWER_STA;/*register address */
		global_bool_modbus.query_power_state=TRUE;
		//printf("充电机 查询充电机状态\n");
		break;

	case CHANNEL_NUM:
		query_buf[1]=ADDRESS_QUERY_POWER_CHA;/*register address */
		global_bool_modbus.query_power_channel=TRUE;
		//printf("充电机 查询充电机通道\n");
		break;

	default:
		break;
	}
	query_buf[0]=0;/*高地址为0*/

	query_buf[2]=0;
	query_buf[3]=1;/*1 Bytes register num*/

	/*function code =0x03: read*/
	send_modbus_data(POWER_ID, 0x03, query_buf, sizeof (query_buf));
	//send_modbus_charge_toggle_data(POWER_ID, 0x03, query_buf, sizeof (query_buf));
	//printf("充电机 查询充电机数据\n");

	return 0;
}

int write_set_power_supply(struct T_POWER *ptr_set_power, unsigned char data_type)
{
	unsigned char set_buf[4]={0};
	unsigned short power_data=0;

	switch(data_type)
	{
	case VOLTAGE:
		set_buf[1]=ADDRESS_SET_POWER_VOL;/*register address */
		power_data=ptr_set_power->battery_voltage*10;
		//printf("充电机 设置电压\n");
		break;
	case CURRENT:
		set_buf[1]=ADDRESS_SET_POWER_CUR;/*register address */
		power_data=ptr_set_power->work_current*10;
		//printf("充电机 设置电流\n");
		break;
	case  STATE:
		set_buf[1]=ADDRESS_SET_POWER_STA;/*register address */
		power_data=ptr_set_power->work_state;
		//printf("充电机 设置状态\n");
		break;
	case CHANNEL_NUM:
		set_buf[1]=ADDRESS_SET_POWER_CHA;/*register address */
		power_data=ptr_set_power->channel_num;
		//printf("充电机 设置充电通道\n");
		break;

	default:
		break;
	}

	set_buf[0]=0;/*高地址为0*/

	set_buf[2]=(unsigned char)(power_data >> 8 & 0x00FF);
	set_buf[3]=(unsigned char)(power_data & 0x00FF);

	/*function code =0x06: write*/
	send_modbus_data(POWER_ID, 0x06, set_buf, sizeof (set_buf));
	//send_modbus_charge_toggle_data(POWER_ID, 0x06, set_buf, sizeof (set_buf));

	previous_write_type=data_type;
	previous_write_set_power=power_data;

	return 0;
}

int read_power_data(struct T_POWER *ptr_read_power, unsigned char buf[], unsigned int len)
{
	unsigned char high=0;
	unsigned char low=0;
	unsigned short data=0;

	high=buf[0];
	low=buf[1];
	data=(unsigned short)high << 8 | low;
	printf("收到充电机数据\n");

	if(global_bool_modbus.query_power_voltage)
	{
		global_bool_modbus.query_power_voltage=FALSE;

		ptr_read_power->battery_voltage=(unsigned short)((float)data*0.1);
		//printf("充电机 充电机的电压=%d\n",ptr_read_power->battery_voltage);
	}
	else if(global_bool_modbus.query_power_current)
	{
		global_bool_modbus.query_power_current=FALSE;

		ptr_read_power->work_current=(unsigned short)((float)data*0.1);
		//printf("充电机 充电机电流=%d\n",ptr_read_power->work_current);
	}
	else if(global_bool_modbus.query_power_state)
	{
		global_bool_modbus.query_power_state=FALSE;
		ptr_read_power->work_state=data;
		//printf("充电机 充电机状态=%x 0x02--开机  0x03--关机\n",data);

		if((ptr_read_power->work_state >> 0) & 0x0001)
		{
			//printf("充电机 充电机关机状态\n");
			ptr_read_power->power_state.is_off=TRUE;
		}
		else
		{
			//printf("充电机 充电机开机状态\n");
			ptr_read_power->power_state.is_off=FALSE;
		}
		if((ptr_read_power->work_state >> 1) & 0x0001)
		{
			ptr_read_power->power_state.is_manual=TRUE;
		}
		if((ptr_read_power->work_state >> 2) & 0x0001)
		{
			ptr_read_power->power_state.no_battery=TRUE;
		}
		if((ptr_read_power->work_state >> 3) & 0x0001)
		{
			ptr_read_power->power_state.reversal_connected=TRUE;
		}
		if((ptr_read_power->work_state >> 4) & 0x0001)
		{
			ptr_read_power->power_state.over_voltage=TRUE;
		}
		if((ptr_read_power->work_state >> 5) & 0x0001)
		{
			ptr_read_power->power_state.over_current=TRUE;
		}

	}
	else if(global_bool_modbus.query_power_channel)
	{
		global_bool_modbus.query_power_channel=FALSE;

		ptr_read_power->channel_num=data;
		//printf("充电机 充电机目前充电通道=%d\n",ptr_read_power->channel_num-1);//这里-1的目的是为了是0对应充电机的电池1
	}

	return 0;
}

/*
 *1 先确认充电机关机状态
 *2 发送充电通道channel
 *3 检查返回的通道是否与写入的通道一致
 *4 如果通道一致，发送充电电压电流
 *5 发送开机指令
 *6 查询充电信息
 *7 在充电过程中可以修改充电电压/电流等，但是无法切换通道
 *8 满足充电条件后，发送关机指令，切换下一通道
 *
 *
 */
int start_charge_power(struct T_POWER *ptr_query_power,\
		               struct T_POWER *ptr_set_power,\
					   struct T_POWER *ptr_read_power)
{

	write_query_power_supply(ptr_query_power, STATE);

#if 0
	/*
	 * 按照说明应该是必须关机情况下才能充电，
	 * 这个关机指的是没有开始充电
	 * 充电结束后，才能切换通道，也就是每次loop循环只能给一个通道充电
	 */
	//正是因为这么一句，所以跟费老师的模拟机测试时，如果一旦结束了自驾程序，模拟机必须重启，否则无法充电
	if(ptr_read_power->power_state.is_off)
	{
		write_set_power_supply(ptr_set_power, CHANNEL_NUM);
	}
#endif

	/*充电开始，先设置充电号，然后查询该充电号的电压，和检查充电号是否已经写入*/
	write_set_power_supply(ptr_set_power, CHANNEL_NUM);

	write_query_power_supply(ptr_query_power, VOLTAGE);
	write_query_power_supply(ptr_query_power, CURRENT);
	write_query_power_supply(ptr_query_power, CHANNEL_NUM);/*确保充电机的当前工作充电号，的确是，将要充电的充电号*/

	//if(!ptr_read_power->power_state.no_battery)
	if(1)
	{
		if(ptr_set_power->channel_num == ptr_read_power->channel_num)
		{
			//printf("充电机 充电机目前充电通道（设定的通道跟正在充电的通道相同）=%d\n",ptr_set_power->channel_num-1);//这里-1，为了对应0为充电机的1号电池

			if(ptr_set_power->battery_voltage > ptr_read_power->battery_voltage)
			{
				if(global_bool_modbus.power_set_voltage_done)
				{

				}
				else
				{
					//printf("充电机 设置电压 > 电池电压，需要充电\n");
					write_set_power_supply(ptr_set_power, VOLTAGE);
					ptr_set_power->work_current=10;
					write_set_power_supply(ptr_set_power, CURRENT);

					global_bool_modbus.power_set_voltage_done=TRUE;
				}

				if(ptr_read_power->power_state.is_off)
				{
					ptr_set_power->work_state=0;/*开机*/
					write_set_power_supply(ptr_set_power, STATE);/*开机 打开电源充电*/
				}
				else
				{

				}

				return 0;
			}
			//else if(ptr_set_power->battery_voltage <= ptr_read_power->battery_voltage)
			//修改为电流判断，实际的充电电流小于了设定的停止充电电流，就该停止充电了
			else if(ptr_set_power->work_current >= ptr_read_power->work_current)
			{
				/*进行2次关机操作，尽量确保充电完毕后就关机*/
				ptr_set_power->work_state=1;/*关机*/
				write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

				ptr_set_power->work_state=1;/*关机*/
				write_set_power_supply(ptr_set_power, STATE);/*关机 关闭充电*/

				global_bool_modbus.power_set_voltage_done=FALSE;

				return 1;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 0;
}


