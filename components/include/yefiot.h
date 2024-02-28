#ifndef _YEFIOT_H_
#define _YEFIOT_H_
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

// #include "debug.h"

// #include "XQDB.h"
// #include "XQUart.h"


class yf_param{
private:

public:
	int open_delay;
	char opendoor_psw[6];
    char set_psw[10];
	
	yf_param();
};

int handle_key_button(int mode);




#endif