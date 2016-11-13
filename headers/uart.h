/*
 * uart.h
 *
 *  Created on: Oct 12, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_UART_H_
#define HEADERS_UART_H_

#if 0
#define UART_DEV_TOTAL 20
#define UART_BUF_SIZE 512

struct T_UART_DEVICE_PROPERTY
{
	unsigned char uart_num;
	unsigned char databits;
	unsigned char parity;
	unsigned char stopbits;

	unsigned int baudrate;
};

extern int uart_fd[UART_DEV_TOTAL];
extern char *uart_dev[];
extern struct T_UART_DEVICE_PROPERTY uart_device_property;

int open_uart_dev(int fd, int uart_no);
int set_uart_opt(int fd, int speed, int bits, char event, int stop);
int read_uart_data(int fd, char *rcv_buf, int time_out_ms, int buf_len);
int send_uart_data(int fd, char *send_buf, int buf_len);

void uart_recvbuf_and_process(int uart_no);
int uart_device_pthread(int uart_no);

/*
 * 测试时，保存读取的数据
 */
/*int dataSave(int uart_no, int baudrate);*/
#else
struct T_UART_DEVICE
{
    char *uart_name;
    int (*ptr_fun)(unsigned char *buf,unsigned int len);

    unsigned char uart_num;

    unsigned char databits;
    unsigned char parity;
    unsigned char stopbits;
    unsigned int  baudrate;
};

extern struct T_UART_DEVICE uart_device;

/*
 * 根据串口的名称，确定在uart_dev[]中的编号，比如/dev/ttyO0对应0，/dev/ttyO1对应1
 * 返回值是串口的编号
 */
int open_uart_dev(char *uart_name);
int set_uart_opt(char *uart_name, int speed, int bits, char event, int stop);
int read_uart_data(char *uart_name, char *rcv_buf, int time_out_ms, int buf_len);
int send_uart_data(char *uart_name, char *send_buf, int buf_len);
int close_uart_dev(char *uart_name);

/*
 * 创建串口线程
 * uart_recvbuf_and_process是串口线程中的处理函数，参数为ptr_uart_device
 * 处理函数必须是int ptr_fun (unsigned char *buf,unsigned int len);类型的函数
 */
int create_uart_pthread(struct T_UART_DEVICE *ptr_uart_device);
void uart_recvbuf_and_process(void * ptr_uart_device);
#endif

#endif /* HEADERS_UART_H_ */
