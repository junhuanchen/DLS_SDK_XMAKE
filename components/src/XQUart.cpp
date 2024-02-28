/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XQUart.cpp
 * Author: xiafeng
 * 
 * Created on January 24, 2022, 6:38 PM
 */

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <time.h>
#include <sys/stat.h>  
#include <fcntl.h>  
#include <termios.h>  
#include <stdio.h> 
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif
#include "XQUart.h"

XQUart::XQUart() {

    // m_pWnd = NULL;
    m_fd = 0;


}

XQUart::~XQUart() {



}

int XQUart::set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio, oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(fd, &oldtio) != 0) {
        fprintf(stderr, "SetupSerial 1");
        fprintf(stderr, "tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof ( newtio));
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch (nBits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }
    /*设置奇偶校验位*/
    switch (nEvent) {
        case 'o':
        case 'O': //奇数   
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'e':
        case 'E': //偶数   
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'n':
        case 'N': //无奇偶校验位   
            newtio.c_cflag &= ~PARENB;
            break;
        default:
            break;
    }
    /*设置波特率*/
    switch (nSpeed) {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 38400:
            cfsetispeed(&newtio, B38400);
            cfsetospeed(&newtio, B38400);
            break;
        case 57600:
            cfsetispeed(&newtio, B57600);
            cfsetospeed(&newtio, B57600);
            break;    
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    /*设置停止位*/
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd, TCIFLUSH);
    /*激活新配置*/
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        fprintf(stderr, "com set error");
        return -1;
    }
    // fprintf(stderr, "set done!\n");
    return 0;
}

int XQUart::open_portByString(char *szName) {

    int fd = open(szName, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "open serial port\n");
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0) {
        fprintf(stderr, "fcntl F_SETFL\n");
        return -1;
    }
    //  if(isatty(STDIN_FILENO) == 0)  
    //  {  
    //         fprintf(stderr,"standard input is not a terminal device");  
    //         return -1;  
    //  }  
    return fd;
}

bool XQUart::OpenUart(int idx, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {

// printf("*********************\n");
    char sUart[50];
    sprintf(sUart, "/dev/ttyS%d", idx);
    
    return OpenUart(sUart, nSpeed, nBits, nEvent, nStop);
}

bool XQUart::OpenUart(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {
// printf("#######################\n");
    m_fd = open_portByString(sUart);
    if (m_fd < 0) {
        return false;
    }
    // if(set_opt(m_fd, 115200, 8, 'N', 1) < 0){
    if (set_opt(m_fd, nSpeed, nBits, nEvent, nStop) < 0) {

        printf("set_com_config error");
        close(m_fd);
        return false;
    }
    return true;
}

void XQUart::CloseUart() {
    if (m_fd) {
        close(m_fd);
        m_fd = 0;
    }

}

int XQUart::Recv(char *pData, int size) {
    if (m_fd == 0) {
        fprintf(stderr, "Open:Read Uart  Error\n");
        return 0;
    }
    int res = read(m_fd, pData, size);
    return res;

}

int XQUart::Send(char *pData, int size) {

    if (m_fd == 0) {
        fprintf(stderr, "Open:Write Uart  Error\n");
        return 0;
    }
    int res = write(m_fd, pData, size);
    if (res == 0) {
        fprintf(stderr, "Write Uart read Error\n");
        return 0;
    }
    return res;
}

void XQUart::run() {

    return;
}

/*******************************************************************************
 *函数名：  wait_uart_data
 *功能	：    等待是否存在串口数据
 *入口参数：time_out：超时时�?

 *返回参数：有数据 0：无数据
 *作者： Frank
 *******************************************************************************/
int XQUart::wait_uart_data(uint8_t time_out) {

    int fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(m_fd, &fs_read);

    time.tv_sec = time_out;
    time.tv_usec = 0;

    //判断是否存在数据
    fs_sel = select(m_fd + 1, &fs_read, NULL, NULL, &time);
    //  printf("fs_sel = %d\r\n",fs_sel);  
    if (fs_sel > 0) { //更正为负数的情况
        return 1;
    }
    return -1;
}

int XQUart::uart_read_data_noblock(uint8_t *buf, int len, uint16_t time_out) {
    int ret;
    uint16_t j = 0;
    uint8_t rcv_buf;
    uint16_t try_cnt = 0;

    for (j = 0; j < len; j++) {
        try_cnt = 0;
        do {
            ret = read(m_fd, &rcv_buf, 1);
        } while (ret <= 0 && (try_cnt++ <time_out));
        if (ret <= 0) {
            break;
        }
        buf[j] = rcv_buf;

    }
    if (j <= 0) {
        return -1;
    }
    return j;
}

int XQUart::uart_read_bytes(uint8_t *buffer, int len, uint16_t time_out) {
    int total_bytes_read = 0;
    int bytes_to_read = len;

    while (total_bytes_read < bytes_to_read) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_fd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = time_out;  // 设置超时时间为5秒
        timeout.tv_usec = 0;

        int ret = select(m_fd + 1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) {
            return -1;
        } else if (ret == 0) {  // timeout 
            return -2;
        } else {
            if (FD_ISSET(m_fd, &readfds)) {
                int num_bytes = read(m_fd, buffer + total_bytes_read, bytes_to_read - total_bytes_read);
                if (num_bytes > 0) {
                    total_bytes_read += num_bytes;
                }
            }
        }
    }

    if (total_bytes_read == bytes_to_read) {
        return total_bytes_read;
    } else {
        // printf("Failed to read the expected number of bytes\n");
        return -3;
    }
}
int XQUart::uart_flush()
{
    return tcflush(m_fd, TCIFLUSH); 
}

XQUart1::XQUart1() {

    // m_pWnd = NULL;
    m_fd1 = 0;
}
XQUart1::~XQUart1() {

}

int XQUart1::set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio, oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(fd, &oldtio) != 0) {
        fprintf(stderr, "SetupSerial 1");
        fprintf(stderr, "tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof ( newtio));
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch (nBits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }
    /*设置奇偶校验位*/
    switch (nEvent) {
        case 'o':
        case 'O': //奇数   
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'e':
        case 'E': //偶数   
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'n':
        case 'N': //无奇偶校验位   
            newtio.c_cflag &= ~PARENB;
            break;
        default:
            break;
    }
    /*设置波特率*/
    switch (nSpeed) {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 38400:
            cfsetispeed(&newtio, B38400);
            cfsetospeed(&newtio, B38400);
            break;
        case 57600:
            cfsetispeed(&newtio, B57600);
            cfsetospeed(&newtio, B57600);
            break;    
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    /*设置停止位*/
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd, TCIFLUSH);
    /*激活新配置*/
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        fprintf(stderr, "com set error");
        return -1;
    }
    // fprintf(stderr, "set done!\n");
    return 0;
}

int XQUart1::open_portByString(char *szName) {

    int fd = open(szName, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "open serial port\n");
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0) {
        fprintf(stderr, "fcntl F_SETFL\n");
        return -1;
    } 
    return fd;
}

bool XQUart1::OpenUart(int idx, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {
    char sUart[50];
    sprintf(sUart, "/dev/ttyS%d", idx);
    
    return OpenUart(sUart, nSpeed, nBits, nEvent, nStop);
}

bool XQUart1::OpenUart(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {
    m_fd1 = open_portByString(sUart);
    if (m_fd1 < 0) {
        return false;
    }
    if (set_opt(m_fd1, nSpeed, nBits, nEvent, nStop) < 0) {

        printf("set_com_config error");
        close(m_fd1);
        return false;
    }
    return true;
}

void XQUart1::CloseUart() {
    if (m_fd1) {
        close(m_fd1);
        m_fd1 = 0;
    }

}

int XQUart1::Send(char *pData, int size) {

    if (m_fd1 == 0) {
        fprintf(stderr, "Open:Write Uart  Error\n");
        return 0;
    }
    int res = write(m_fd1, pData, size);
    if (res == 0) {
        fprintf(stderr, "Write Uart read Error\n");
        return 0;
    }
    return res;
}

void XQUart1::run() {

    return;
}

int XQUart1::uart_read_bytes(uint8_t *buffer, int len, uint16_t time_out) {
    int total_bytes_read = 0;
    int bytes_to_read = len;

    while (total_bytes_read < bytes_to_read) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_fd1, &readfds);

        struct timeval timeout;
        timeout.tv_sec = time_out;  // 设置超时时间为5秒
        timeout.tv_usec = 0;

        int ret = select(m_fd1 + 1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) {
            return -1;
        } else if (ret == 0) {  // timeout 
            return -2;
        } else {
            if (FD_ISSET(m_fd1, &readfds)) {
                int num_bytes = read(m_fd1, buffer + total_bytes_read, bytes_to_read - total_bytes_read);
                if (num_bytes > 0) {
                    total_bytes_read += num_bytes;
                }
            }
        }
    }

    if (total_bytes_read == bytes_to_read) {
        return total_bytes_read;
    } else {
        // printf("Failed to read the expected number of bytes\n");
        return -3;
    }
}
int XQUart1::uart_flush()
{
    return tcflush(m_fd1, TCIFLUSH); 
}