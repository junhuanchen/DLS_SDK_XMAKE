
#ifndef UARTFINGER_H
#define UARTFINGER_H
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "XQThread.h"
// #include "XQWnd.h"

class UartFinger:public XQThread {
public:
    UartFinger();
    virtual ~UartFinger();
private:
    int m_fd;
private:
    int  open_portByString(char  *szName); 
    int  set_opt(int fd,int nSpeed, 
         int nBits, 
         char nEvent, 
         int nStop); 
    virtual void run();
public:
     bool OpenUartFinger(int idx,int nSpeed, 
         int nBits, 
         char nEvent, 
         int nStop);   
     
     bool OpenUartFinger(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop);
     void CloseFinger();
     
     int Recv(char *pData,int size);
     int Send(char *pData,int size);
     int Recv(uint8_t *pData,int size);
     int Send(uint8_t *pData,int size);

    //  int wait_uart_data(uint8_t time_out) ;
    //  int uart_read_data_noblock(uint8_t *buf, int len, uint16_t time_out);
     int uart_read_bytes(uint8_t *buf, int len, uint16_t time_out);
     int uart_flush();
protected:
    //   XQWnd  *m_pWnd;
};
// class XQUart1:public XQThread {
// public:
//     XQUart1();
//     virtual ~XQUart1();
// private:
//     int m_fd1;
// private:
//     int  open_portByString(char  *szName); 
//     int  set_opt(int fd,int nSpeed, 
//          int nBits, 
//          char nEvent, 
//          int nStop); 
//     virtual void run();
// public:
//      bool OpenUart(int idx,int nSpeed, 
//          int nBits, 
//          char nEvent, 
//          int nStop);   
     
//      bool OpenUart(char *sUart, int nSpeed,
//         int nBits,
//         char nEvent,
//         int nStop);
//      void CloseUart();
     
//      int Recv(char *pData,int size);
//      int Send(char *pData,int size);

//      int uart_read_bytes(uint8_t *buf, int len, uint16_t time_out);
//      int uart_flush();
// protected:
// };

#endif /* UARTFINGER_H */

