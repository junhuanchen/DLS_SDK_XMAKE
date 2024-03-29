#ifndef EVENT_H
#define EVENT_H
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

// #include "UartFinger.h"
// #include "finger.h"
#include "yefiot.h"
// #include "XQThread.h"

// event *g_event = NULL;

class event:public XQThread
{
private:
    int input_event0;
    bool ev_open;
    bool event_init();
public:
    event();
    virtual ~event();

public:
    int c;
    void event_deinit();
    int keycode_to_password(int keycode);
private:
    int state;
    int function;
    virtual void run();
};









// int linux_event_main();






#endif /* EVENT_H */