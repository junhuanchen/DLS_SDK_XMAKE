#ifndef FINGER_H
#define FINGER_H
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
// #include "uart.h"
#include "UartFinger.h"

#pragma pack(1)
typedef struct _uart_head{
        uint16_t head; 
        uint32_t addr;
        uint8_t flag;
        uint16_t headlen;
		uint8_t confir;
    } UartHead;

#pragma pack()

#pragma pack(1)
typedef struct _uart_head1{
        uint16_t head; 
        uint32_t addr;
        uint8_t flag;
        uint16_t bodylen;
		// uint8_t body[300];
    } UartHead1;

#pragma pack()

// typedef struct{
// 	int baud;
// 	int data_bits;
// 	int stop_bits;
// 	char parity;
// }uart_t;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// finger *g_finger = NULL;

class finger:public UartFinger
{
public:
    finger();
    ~finger();
private:
    bool OpenFinger();
private:
    bool m_bOpen;
public:
    int finger_status;
    int read_pkg(uint8_t *buf);
    uint16_t get_chk(uint8_t*data,int len);
    int SendToUart(uint8_t confir,uint8_t flag, uint8_t* body, uint16_t len,uint8_t *redata);

    int Register();
    int ConfirFinger();
    int GetFeature(int times);
    int Combine();
    int SaveFinger(int finger_id);
    int SearchFinger(int &fingerid);
    int Delete_Finger(int finger_id);
    int DeleteAllFinger();

    // int uart_load();
    int confir();
    int logon(int fg_id,int &fingerid);



private:
    virtual void run();
};


#endif /* FINGER_H */