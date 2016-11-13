/*
 * modbus_toggle_discharge.h
 *
 *  Created on: Jul 23, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_MODBUS_TOGGLE_DISCHARGE_H_
#define HEADERS_MODBUS_TOGGLE_DISCHARGE_H_


/*
 * data type
 */
#define TOGGLE_BATTERY_VOLTAGE0 0x00
#define TOGGLE_BATTERY_STATE0 0x01
#define TOGGLE_BATTERY_VOLTAGE1 0x02
#define TOGGLE_BATTERY_STATE1 0x03

#define TOGGLE_CHARGING_STATE 0x10

#define TOGGLE_BATTERY_LOW_LIMIT 0x20
#define TOGGLE_BATTERY_HIGH_LIMIT 0x21
#define TOGGLE_DISCHARGE_NUM 0x22

/*
 * register address
 * every "data type" has a query or set "register address"
 * 每个data type都有一个寄存器访问地址
 * ADDRESS_QUERY is for query device
 * ADDRESS_SET is for set device
 */
#define ADDRESS_QUERY_TOGGLE_BATTERY_VOL0 0x00
#define ADDRESS_QUERY_TOGGLE_BATTERY_STA0 0x01
#define ADDRESS_QUERY_TOGGLE_BATTERY_VOL1 0x02
#define ADDRESS_QUERY_TOGGLE_BATTERY_STA1 0x03

#define ADDRESS_SET_TOGGLE_CHARGING_STA 0x10
#define ADDRESS_SET_TOGGLE_BAT_LOW_LIMIT 0x20
#define ADDRESS_SET_TOGGLE_BAT_HIGH_LIMIT 0x21
#define ADDRESS_SET_TOGGLE_DISCHARGE_NUM 0X22

struct T_TOGGLE
{
	unsigned short battery0_voltage;
	unsigned short battery0_state;
	unsigned short battery1_voltage;
	unsigned short battery1_state;

	unsigned short charging_state;
	unsigned short battery_low_limit;
	unsigned short battery_high_limit;
	unsigned short discharge_num;
};

extern struct T_TOGGLE query_toggle;
extern struct T_TOGGLE set_toggle;
extern struct T_TOGGLE read_toggle;

int set_init_toggle_discharge();

int write_query_toggle_discharge(struct T_TOGGLE *ptr_query_toggle, unsigned char data_type);
int write_set_toggle_discharge(struct T_TOGGLE *ptr_set_toggle, unsigned char data_type);
int read_toggle_data(struct T_TOGGLE *ptr_read_toggle, unsigned char buf[], unsigned int len);

int toggle_discharge_loop(struct T_TOGGLE *ptr_query_toggle,
		                  struct T_TOGGLE *ptr_set_toggle, \
						  struct T_TOGGLE *ptr_read_toggle);

#endif /* HEADERS_MODBUS_TOGGLE_DISCHARGE_H_ */
