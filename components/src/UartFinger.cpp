
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
#include "UartFinger.h"

UartFinger::UartFinger() {

    // m_pWnd = NULL;
    m_fd = 0;
}
UartFinger::~UartFinger() {

}

int UartFinger::set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio, oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    // if (tcgetattr(fd, &oldtio) != 0) {
    //     fprintf(stderr, "SetupSerial 1");
    //     fprintf(stderr, "tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
    //     return -1;
    // }
    printf("tcgetattr( fd,&oldtio):%d\n",tcgetattr(fd, &oldtio));
    bzero(&newtio, sizeof ( newtio));
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    // newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch (nBits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        default:break;
    }
    /*设置奇偶校验位*/
    switch (nEvent) {
        case 'o':
        case 'O': //奇数   
            // newtio.c_cflag |= PARENB;
            // newtio.c_cflag |= PARODD;
            newtio.c_cflag |= (PARODD | PARENB);
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
            newtio.c_iflag &= ~INPCK;
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
    switch (nStop)
    {
    case 1:
        newtio.c_cflag &= ~CSTOPB;
        break;
    case 2:
        newtio.c_cflag |= CSTOPB;
        break;
    default:break;
    }
    // if (nStop == 1)
    //     newtio.c_cflag &= ~CSTOPB;
    // else if (nStop == 2)
    //     newtio.c_cflag |= CSTOPB;

     /* 设置流控制 */
    newtio.c_cflag &= ~CRTSCTS;

    /*设置等待时间和最小接收字符*/
    // newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;

    /* 刷新串口，更新配置 */
    tcflush(fd, TCIOFLUSH);
    // int res = tcsetattr(fd, TCSANOW, &newtio);
    /*处理未接收字符*/
    // tcflush(fd, TCIFLUSH);
    /*激活新配置*/
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        fprintf(stderr, "com set error");
        return -1;
    }
    fprintf(stderr, "set done!\n");
    return 0;
}

int UartFinger::open_portByString(char *szName) {

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

bool UartFinger:: OpenUartFinger(int idx, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {
    char sUart[50];
    sprintf(sUart, "/dev/ttyS%d", idx);
    
    return OpenUartFinger(sUart, nSpeed, nBits, nEvent, nStop);
}

bool UartFinger::OpenUartFinger(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop) {
    m_fd = open_portByString(sUart);
    // printf("open  m_fd:%d\n",m_fd);
    if (m_fd < 0) {
        return false;
    }
    if (set_opt(m_fd, nSpeed, nBits, nEvent, nStop) < 0) {

        printf("set_com_config error");
        close(m_fd);
        return false;
    }
    return true;
}

void UartFinger::CloseFinger() {
    if (m_fd) {
        close(m_fd);
        m_fd = 0;
    }
}

int UartFinger::Recv(char *pData, int size) {
    if (m_fd == 0) {
        fprintf(stderr, "Open:Read Uart  Error\n");
        return 0;
    }
    int res = read(m_fd, pData, size);
    printf("rev:%d\n",res);
    printf("pData:%s\n",pData);
    return res;

}
int UartFinger::Send(char *pData, int size) {

    if (m_fd == 0) {
        fprintf(stderr, "Open:Write Uart  Error\n");
        return 0;
    }
    int res = write(m_fd, pData, size);
    // printf("res:%d\n",res);
    if (res == 0) {
        fprintf(stderr, "Write Uart read Error\n");
        return 0;
    }
    return res;
}

void UartFinger::run() {

    return;
}

int UartFinger::uart_read_bytes(uint8_t *buffer, int len, uint16_t time_out) {
    int total_bytes_read = 0;
    int bytes_to_read = len;

    while (total_bytes_read < bytes_to_read) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_fd, &readfds);
        // printf("m_fd:%d\n",m_fd);
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
int UartFinger::uart_flush()
{
    return tcflush(m_fd, TCIFLUSH); 
}