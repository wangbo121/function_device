/*
 * modbus_485.h
 *
 *  Created on: 2016年5月18日
 *      Author: wangbo
 */

#ifndef HEADERS_MODBUS_485_H_
#define HEADERS_MODBUS_485_H_

#define POWER_ID 0x01
#define ANALOG_ID 0x02
#define RELAY_SWITCH_ID 0x04
#define ROTARY_ENCODER_ID 0x08
#define TOGGLE_DISCHARGE_ID 0x03

#define UART_MODBUS_BAUD 9600
#define UART_MODBUS_DATABITS 8 //8 data bit
#define UART_MODBUS_STOPBITS 1 //1 stop bit
#define UART_MODBUS_PARITY 0 //no parity

//#define MODBUS_MAX_WAIT_TIME 0.6//0.6s=600ms
#define MODBUS_MAX_WAIT_TIME 0.5//0.6s=600ms

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct T_GLOBAL_BOOL_MODBUS
{
	unsigned char is_using_modbus;
	unsigned char modbus_all_device_query_cnt;
	unsigned char modbus_is_querying_device;

	unsigned char modbus_is_sending;//只有在发送一帧数据，并且得到相应后，modbus才能继续发送下一数据
	unsigned char modbus_is_sending_id;
	unsigned char modbus_is_sending_func_code;//功能码
#if 0
	long long int modbus_send_time;
	long long int modbus_receive_time;
	unsigned long long int modbus_max_has_waited_time;//ms
	unsigned long long int modbus_send_and_receive_time;//ms
#endif
	float modbus_send_time;
	float modbus_receive_time;
	float modbus_max_has_waited_time;//ms
	float modbus_send_and_receive_time;//ms
	/*
	 * 这个是切换访问不同485设备的频率
	 */
	unsigned char modbus_rotary_and_rudder;
	unsigned char modbus_power;
	unsigned char modbus_toggle;




	unsigned char query_power_voltage;
	unsigned char query_power_current;
	unsigned char query_power_state;
	unsigned char query_power_channel;
	unsigned char write_power_success;
	unsigned char power_charge_cnt;
	unsigned char power_set_voltage_done;
	unsigned char power_query_cnt;
	unsigned char power_able_to_query;
	unsigned char power_battery0_is_charging;
	unsigned char power_battery1_is_charging;

	unsigned char query_analog_channel0;
	unsigned char query_analog_channel1;
	unsigned char query_analog_channel2;
	unsigned char query_analog_channel3;
	unsigned char write_analog_success;

	unsigned char query_relay_switch;
	unsigned char write_relay_switch_success;

	unsigned char query_encoder;
	unsigned char write_encoder_success;

	unsigned char query_toggle_battery_vol0;
	unsigned char query_toggle_battery_sta0;
	unsigned char query_toggle_battery_vol1;
	unsigned char query_toggle_battery_sta1;
	unsigned char write_toggle_charging_sta;
	unsigned char write_toggle_discharge_num;
	unsigned char write_toggle_success;
	unsigned char toggle_query_cnt;
	unsigned char toggle_able_to_query;

	unsigned char toggle_cnt;

	unsigned char toggle_battery0_is_discharging;
	unsigned char toggle_battery0_request_discharge;
	unsigned char toggle_battery0_stop_discharge;
	unsigned char toggle_battery0_is_charging;
	unsigned char toggle_battery0_allow_discharge;
	unsigned char toggle_battery0_request_charge;

	unsigned char toggle_battery1_is_discharging;
	unsigned char toggle_battery1_request_discharge;
	unsigned char toggle_battery1_stop_discharge;
	unsigned char toggle_battery1_is_charging;
	unsigned char toggle_battery1_allow_discharge;
	unsigned char toggle_battery1_request_charge;
};

extern struct T_GLOBAL_BOOL_MODBUS global_bool_modbus;

int sleep_ms(int ms);

int modbus_uart_init();
int read_modbus_data(unsigned char *buf, unsigned int len);
int send_modbus_data(unsigned char dev_id, unsigned char function_code, unsigned char *buf, unsigned int len);
int modbus_uart_close(unsigned int uart_num);

#if 0
int modbus_rotary_uart_init(unsigned int uart_num);
int modbus_charge_toggle_uart_init(unsigned int uart_num);

int read_modbus_rotary_data(unsigned char *buf, unsigned int len);
int read_modbus_charge_toggle_data(unsigned char *buf, unsigned int len);

int send_modbus_rotary_data(unsigned char dev_id, unsigned char function_code, unsigned char *buf, unsigned int len);
int send_modbus_charge_toggle_data(unsigned char dev_id, unsigned char function_code, unsigned char *buf, unsigned int len);
#endif

#endif /* HEADERS_MODBUS_485_H_ */
