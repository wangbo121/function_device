/*
 * udp.c
 *
 *  Created on: Nov 13, 2016
 *      Author: wangbo
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<stdarg.h>
#include<string.h>

#include "udp.h"

int fd_sock_send;
int fd_sock_recv;

struct T_UDP_DEVICE udp_device;

static struct sockaddr_in udp_myrecv_addr;//服务器用于接收的socket
static struct sockaddr_in udp_sendto_addr;//服务器用于发送的socket
static struct sockaddr_in client_addr;//服务器用来保存客户端的发送socket，把客户端的socket属性保存在client_addr

int open_udp_dev(char* ip_sendto, int port_sendto, int port_myrecv)
{
    /*
     * 指定了发送目标的端口为port_sendto，也就是如果要发送给服务器，必须知道服务器哪个端口是会处理我的数据
     * 指定了我这个程序的接收端口，既然服务器的端口是比如6789，那么我就也把我的接收端口设置为6789呗，从而统一个数据接收和发送
     * 整体来说，发送时指定对方ip地址，接收来自任意ip的数据
     * 发送时不需要绑定端口，系统会自动分配
     * 接收时绑定端口，作为服务器肯定是固定端口接收，当然了其实客户端也可以固定端口发送的
     */

    int sockaddr_size;
    /*设置发送目标的ip地址和发送目标的端口*/
    sockaddr_size = sizeof(struct sockaddr_in);
    bzero((char*)&udp_sendto_addr, sockaddr_size);
    udp_sendto_addr.sin_family = AF_INET;
    udp_sendto_addr.sin_addr.s_addr = inet_addr(ip_sendto);
    udp_sendto_addr.sin_port = htons(port_sendto);
    /*建立“发送”套接字*/
    fd_sock_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_sock_send == -1)
    {
        perror("udp send create socket failed");
        close(fd_sock_send);
        return 0;
    }
    else
    {
        //printf("udp send ini ok!\n");
    }

    /*设置本机的接收ip地址和端口，但是接收的ip不指定，因为任何的ip都有可能给我发数据呀*/
    sockaddr_size = sizeof(struct sockaddr_in);
    bzero((char*)&udp_myrecv_addr, sockaddr_size);
    udp_myrecv_addr.sin_family = AF_INET;
    udp_myrecv_addr.sin_addr.s_addr = INADDR_ANY;//任意地址
    udp_myrecv_addr.sin_port = htons(port_myrecv);
    /*建立“接收”套接字*/
    fd_sock_recv = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_sock_recv == -1)
    {
        perror("udp recv create socket failed");
        close(fd_sock_recv);
        return 0;
    }
    else
    {

    }
    /*
     * 建立socket获取fd_sock文件描述符后，需要把这个文件描述符与某一个soucket绑定
     * 因为socket函数只是建立了一个通用的针对某一地址族的文件描述符，但是该socket的属性还需要设置并且绑定
     */
    if (bind(fd_sock_recv, (struct sockaddr *)&udp_myrecv_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("udp recv bind err");
        close(fd_sock_recv);
        return 0;
    }
    else
    {
        printf("udp recv ini ok!\n");
    }

    int ret=0;
    udp_device.ptr_fun=read_udp_data;
    /*udp_recvbuf_and_process函数中调用了fd_sock_recv来确定从哪个socket获取数据*/
    ret = pthread_create (&udp_device.fd,            //线程标识符指针
                          NULL,                            //默认属性
                          (void *)udp_recvbuf_and_process,//运行函数
                          (void *)&udp_device);                 //运行函数的参数
    if (0 != ret)
    {
       perror ("pthread create error\n");
    }

    return 0;
}

//#define UDP_RCV_BUF_SIZE 2000
#define UDP_RCV_BUF_SIZE 512

#define UDP_PACKET_HEAD 0xa55a5aa5
int send_udp_data(unsigned char *buf, unsigned int len)
{
    int m_head;
    int m_len;
    int ret=0;
    //unsigned char m_sendbuf[UDP_RCV_BUF_SIZE];
    static unsigned char m_sendbuf[UDP_RCV_BUF_SIZE]={'\0'};

    static unsigned int m_packcnt = 0;

    m_head = htonl(UDP_PACKET_HEAD);
    memcpy(m_sendbuf, &m_head, 4);
    m_packcnt++;
    memcpy(&m_sendbuf[4], &m_packcnt, 4);
    memcpy(&m_sendbuf[8], &len, 4);
    memcpy(&m_sendbuf[12], buf, len);
    m_len = 12 + len;
    ret = sendto(fd_sock_send, m_sendbuf, m_len, 0, (struct sockaddr *)&udp_sendto_addr, sizeof(struct sockaddr_in));
    printf("发送的字节数=%d\n",ret);

    return 0;
}


int read_udp_data(unsigned char *buf, unsigned int len)
{
    static unsigned char _buffer[UDP_RCV_BUF_SIZE]={'\0'};
#if 0
    static unsigned char _pack_recv_type;
    static int _pack_recv_cnt = 0;
    static int _pack_recv_len = 0;
    static unsigned char _pack_recv_buf[UDP_RCV_BUF_SIZE];
    static int _pack_buf_len = 0;
    static unsigned char _checksum = 0;

    int _length;
    unsigned char c;
    unsigned char _sysid;

    _length = len;
#endif
    memcpy(_buffer, buf, len);

    /*显示通过udp收到的数据*/
#if 1
    int i = 0;
    printf("udp data buf=\n");
    for(i=0;i<len;i++)
    {
        printf("%c",buf[i]);
    }
    printf("\n");
#endif



    return 0;
}

void udp_recvbuf_and_process(void * ptr_udp_device)
{
    char buf[UDP_RCV_BUF_SIZE] = { 0 };
    unsigned int read_len;

    struct T_UDP_DEVICE *ptr_udp;
    ptr_udp=(struct T_UDP_DEVICE *)ptr_udp_device;

    unsigned int client_addr_len=0;
    client_addr_len=sizeof(struct sockaddr);

    while(1)
    {
        if(-1!=(read_len=recvfrom(fd_sock_recv, buf, 2000, 0, (struct sockaddr *)&client_addr, &client_addr_len)))
        {
#if 0
            printf("read_len=%d\n",read_len);
            buf[read_len]='\0';
            printf("%s\n",buf);
#endif
            if(read_len>0)
            {
                ptr_udp->ptr_fun((unsigned char*)buf,read_len);
            }
        }
    }
    pthread_exit(NULL);

    return ;
}


