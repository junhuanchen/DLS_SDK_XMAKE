#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdio.h>
#include "stdint.h"
#include <unistd.h>
#include <iconv.h>
#include <list>
#include <memory>
#include <pthread.h>
#include <netinet/in.h>
#include <linux/input.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <time.h>

#include "UartFinger.h"
#include "plamer.h"
#include "yefiot.h"


plamer::plamer():UartFinger()
{
	m_bOpen = OpenPlamer();
}
plamer::~plamer()
{
	CloseFinger();
}
bool plamer::OpenPlamer()
{
	int braud = 115200;
	bool b = OpenUartFinger(2,braud,8,'N',1);
	if(b) 
		printf("open plamer success!\n");
	else 
		printf("open plamer fail!\n");
	return b;
}

uint8_t plamer::get_chk(uint8_t*data,int len)
{
    uint8_t checksum = 0x00;
	for (int i = 0; i < len; i++) // 
	{
		checksum += *data;
		data++;
	}
	return checksum;
}
/******************************************获取模组状态******************************
 **************************************************************************************/
int plamer::get_status()
{
    typedef struct plam_get_status{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x11};
        uint8_t datasize[2] = {0x00,0x00};
        uint8_t checksum[1] = {0x00};
    } p_get_status;
	p_get_status param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.datasize));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_get_status_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
        uint8_t stasus[1];
        uint8_t checksum[1];
    } p_get_status_response;
}
/******************************************识别掌静脉******************************
 **************************************************************************************/
int plamer::identify_plamer()
{
	typedef struct plam_identify{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x12};
        uint8_t reser[1] = {0x00};
		uint8_t timeout[1] ={0x03};
        uint8_t checksum[1] = {0x00};
    } p_identify;
	p_identify param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.reser)+sizeof(param.timeout));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_identify_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
		uint8_t user_id[2];
		uint8_t name[32];
		uint8_t admin[1];
        uint8_t stasus[1];
        uint8_t checksum[1];
    } p_identify_response;
}
/******************************************清空注册状态******************************
 **************************************************************************************/
int plamer::clear_enroll()
{
	typedef struct plam_clear_enroll{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x23};
        uint8_t datasize[2] = {0x00,0x00};
        uint8_t checksum[1] = {0x00};
    } p_clear_enroll;
	p_clear_enroll param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.datasize));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_clear_enroll_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
        uint8_t data[1];
        uint8_t checksum[1];
    } p_clear_enroll_response;
}
/******************************************录入掌静脉******************************
 **************************************************************************************/
int plamer::enter_plamer()
{
	typedef struct plam_enter{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x13};
        uint8_t datasize[2] = {0x00,0x23};
		uint8_t admin[1] = {0x00};
		uint8_t name[32] = {};
		uint8_t stage[1] = {0x00};
		uint8_t timeout[1]={0x10};
        uint8_t checksum[1] = {0x00};
    } p_enter;
	p_enter param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param)-3);
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_enter_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
		uint8_t user_id[2];
        uint8_t reser[1];
        uint8_t checksum[1];
    } p_enter_response;
}
/******************************************删除单个用户******************************
 **************************************************************************************/
int plamer::delete_plamer()
{
	typedef struct plam_delete{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x20};
		uint8_t datasize[2] = {0x00,0x02};
        uint8_t user_id_h[1] = {0x00};
		uint8_t user_id_i[1] ={0x00};
        uint8_t checksum[1] = {0x00};
    } p_delete;
	p_delete param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.datasize));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_delete_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
        uint8_t data[1];
        uint8_t checksum[1];
    } p_delete_response;
}
/******************************************删除所有用户******************************
 **************************************************************************************/
int plamer::delete_all_plamer()
{
	typedef struct plam_delete_all{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x21};
        uint8_t datasize[2] = {0x00,0x00};
        uint8_t checksum[1] = {0x00};
    } p_delete_all;
	p_delete_all param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.datasize));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_delete_all_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
        uint8_t admin[1];
        uint8_t checksum[1];
    } p_delete_all_response;
}
/******************************************获取用户数量和ID******************************
 **************************************************************************************/
int plamer::get_plamer_num()
{
	typedef struct plam_get_num{
        uint8_t head[2] = {0xEF,0xAA}; 
        uint8_t code[1] = {0x24};
        uint8_t datasize[2] = {0x00,0x00};
        uint8_t checksum[1] = {0x00};
    } p_get_num;
	p_get_num param;
	uint8_t check = get_chk((uint8_t*)&param.code[0],sizeof(param.code)+sizeof(param.datasize));
	param.checksum[0] = check;
	Send((char*)&param, sizeof(param));

	typedef struct plam_get_num_response{
        uint8_t head[2]; 
		uint8_t flag[1];
		uint8_t datasize[2];
        uint8_t code[1];
		uint8_t confir[1];
        uint8_t counts[1];
		uint8_t user_id[1];
        uint8_t checksum[1];
    } p_get_num_response;
}
void plamer::run()
{
}