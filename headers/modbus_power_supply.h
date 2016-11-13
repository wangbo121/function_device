/*
 * modbus_power_supply.h
 *
 *  Created on: 2016年5月19日
 *      Author: wangbo
 */

#ifndef HEADERS_MODBUS_POWER_SUPPLY_H_
#define HEADERS_MODBUS_POWER_SUPPLY_H_

/*
 * data type
 */
#define VOLTAGE 0x00
#define CURRENT 0x01
#define STATE 0x02
#define CHANNEL_NUM 0x03

/*
 * register address
 * every "data type" has a query or set "register address"
 * 每个data type都有一个寄存器访问地址
 * ADDRESS_QUERY is for query device
 * ADDRESS_SET is for set device
 */
#define ADDRESS_QUERY_POWER_VOL 0x00
#define ADDRESS_QUERY_POWER_CUR 0x01
#define ADDRESS_QUERY_POWER_STA 0x02
#define ADDRESS_QUERY_POWER_CHA 0x03

#define ADDRESS_SET_POWER_VOL 0x10
#define ADDRESS_SET_POWER_CUR 0x11
#define ADDRESS_SET_POWER_STA 0x12
#define ADDRESS_SET_POWER_CHA 0x13

struct T_POWER_STATE
{
	unsigned char is_off;
	unsigned char is_manual;
	unsigned char no_battery;
	unsigned char reversal_connected;
	unsigned char over_voltage;
	unsigned char over_current;
};

struct T_POWER
{
	unsigned short battery_voltage;
	unsigned short work_current;
	unsigned short work_state;
	unsigned short channel_num;

	struct T_POWER_STATE power_state;//8 Bytes
};

#define VOLTAGE_BATTERY0 131
#define VOLTAGE_BATTERY1 131
#define VOLTAGE_BATTERY2 12

#define STOP_CURRENT_BATTERY0 2
#define STOP_CURRENT_BATTERY1 2

extern struct T_POWER query_power;
extern struct T_POWER set_power;
extern struct T_POWER read_power;

int set_init_power_supply();

int write_query_power_supply(struct T_POWER *ptr_query_power, unsigned char data_type);
int write_set_power_supply(struct T_POWER *ptr_set_power, unsigned char data_type);
int read_power_data(struct T_POWER *ptr_read_power, unsigned char buf[], unsigned int len);

int power_charge_loop(struct T_POWER *ptr_query_power,\
		              struct T_POWER *ptr_set_power, \
					  struct T_POWER *ptr_read_power);

#endif /* HEADERS_MODBUS_POWER_SUPPLY_H_ */
