#ifndef PLAM_H
#define PLAM_H
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
#include "UartFinger.h"

#pragma pack(1)
typedef struct _uart_pag{
        uint16_t head; 
        uint8_t code;
        uint16_t datasize;
		uint8_t* data;
        uint8_t check;
    } UartPag;

#pragma pack()

#pragma pack(1)
typedef struct _uart_pag2{
        uint16_t head; 
        uint8_t flag;
        uint16_t datasize;
		// uint8_t body[300];
    } UartPag1;

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

class plamer:public UartFinger
{
public:
    plamer();
    ~plamer();
private:
    bool OpenPlamer();
private:
    bool m_bOpen;
public:
    uint8_t get_chk(uint8_t*data,int len);
    int get_status();
    int identify_plamer();
    int clear_enroll();
    int enter_plamer();
    int delete_plamer();
    int delete_all_plamer();
    int get_plamer_num();
private:
    virtual void run();
};

#endif /* PLAM_H */