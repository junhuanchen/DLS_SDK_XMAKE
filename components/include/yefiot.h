#ifndef _YEFIOT_H_
#define _YEFIOT_H_
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

// #include "debug.h"

#include "XQDB.h"
#include "finger.h"
#include "event.h"
#include "UartFinger.h"


class yf_param{
private:

public:
	int open_delay;
	char opendoor_psw[6];
    char set_psw[10];
	yf_param();
	virtual ~yf_param();

public:
	XQDB *g_pDB;
	int add_userid;
	int finger_uart_main();
	int linux_event_main();
	int finger_opendoor(int fingerid);
	void getparameter();
	void setparameter(char *data);
	int updata_opendoor_psw(char *opendoor_psw);
	int updata_set_psw(char *set_psw);
	int updata_open_delay(int open_delay);
	int open_door();
	int add_user(int user_id);
	int del_user(int user_id = -1);
	int add_user_by_finger(int userid,int fingerid);
	void restore_init();
	void add_record(int user_id,int access_type,int record_time);

private:

	
};
class access
{

};

int handle_key_button(int mode);




#endif