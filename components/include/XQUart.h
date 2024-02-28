/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XQUart.h
 * Author: xiafeng
 *
 * Created on January 24, 2022, 6:38 PM
 */

#ifndef XQUART_H
#define XQUART_H
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

class XQUart:public XQThread {
public:
    XQUart();
    virtual ~XQUart();
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
     bool OpenUart(int idx,int nSpeed, 
         int nBits, 
         char nEvent, 
         int nStop);   
     
     bool OpenUart(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop);
     void CloseUart();
     
     int Recv(char *pData,int size);
     int Send(char *pData,int size);
    //  void SetWnd( XQWnd  * p){
    //      m_pWnd=p;
    //  }
     int wait_uart_data(uint8_t time_out) ;
     int uart_read_data_noblock(uint8_t *buf, int len, uint16_t time_out);
     int uart_read_bytes(uint8_t *buf, int len, uint16_t time_out);
     int uart_flush();
protected:
    //   XQWnd  *m_pWnd;
};
class XQUart1:public XQThread {
public:
    XQUart1();
    virtual ~XQUart1();
private:
    int m_fd1;
private:
    int  open_portByString(char  *szName); 
    int  set_opt(int fd,int nSpeed, 
         int nBits, 
         char nEvent, 
         int nStop); 
    virtual void run();
public:
     bool OpenUart(int idx,int nSpeed, 
         int nBits, 
         char nEvent, 
         int nStop);   
     
     bool OpenUart(char *sUart, int nSpeed,
        int nBits,
        char nEvent,
        int nStop);
     void CloseUart();
     
     int Recv(char *pData,int size);
     int Send(char *pData,int size);

     int uart_read_bytes(uint8_t *buf, int len, uint16_t time_out);
     int uart_flush();
protected:
};

#endif /* XQUART_H */

