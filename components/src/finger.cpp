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
// #include "uart.h"
#include "finger.h"
#include "yefiot.h"

extern finger *g_finger;
extern yf_param *g_yf;
// int baud = 115200;
finger::finger():UartFinger()
{
	finger_status = 0;
	m_bOpen = OpenFinger();
}
finger::~finger()
{
	CloseFinger();
}
bool finger::OpenFinger()
{
	int braud = 57600;
	bool b = OpenUartFinger(2,braud,8,'N',1);
	if(b) 
		printf("open Finger success!\n");
	else 
		printf("open Finger fail!\n");
	return b;
}
uint16_t finger::get_chk(uint8_t*data,int len)
{
	uint16_t sum = 0;
	for (int i = 0; i < len; i++) // 
	{
		sum += *data;
		data++;
	}
	return sum;
}
// UartFinger g_uart;
int finger::read_pkg(uint8_t *buf) {
	uint16_t head;
	uint32_t addr;
	uint8_t flag;
	uint16_t pkglen;
	UartHead *pkghead = (UartHead *)buf;
	uint8_t* body = (uint8_t*)&pkghead->confir;
	uint16_t check_sum1 = 0;
	uint16_t check_sum2 = 0;
	int relen = 0;
	int count = 0;
	while(true) {
		relen = uart_read_bytes((uint8_t*)&head, sizeof(head), 1);
		// printf("relen:%d\n",relen);
		if (relen <= 0) {	
			count++;
			if(count <10)			
				continue;
			else
				return 0;
		}
		if (head != htons(0xEF01)) {
			count++;
			uint8_t* head1 = (uint8_t*)&head;
			printf("包头:%02X %02X\n",head1[0],head1[1]);
			if(count<200)
				continue;
			else	
			{
				printf("can not read head\n");
				count = 0;
				return 0;
			}				
		}
		pkghead->head = head;
		relen = uart_read_bytes((uint8_t*)&addr, sizeof(addr), 1);
		if(relen <= 0) {
			return -1;
		}
		// pkghead->addr = ntohl(addr);
		pkghead->addr = addr;
		relen = uart_read_bytes((uint8_t*)&flag, sizeof(flag), 1);
		if (relen <= 0) {
			return -2;
		}
		pkghead->flag = flag;
		// printf("包标识：%02X\n",flag);
		relen = uart_read_bytes((uint8_t*)&pkglen, sizeof(pkglen), 1);
		if (relen <= 0) {
			return -3;
		}
		pkghead->headlen = pkglen;
		int int_len = (int)ntohs(pkglen);
		// printf("int_len:%d\n",int_len);
		relen = uart_read_bytes(body, int_len, 10);
		if (relen <= 0) {
			return -4;
		}
		check_sum1 = htons(this->get_chk(&pkghead->flag, int_len + 1));
		check_sum2 = *(uint16_t*)&body[int_len - 2];
		if (check_sum1 != check_sum2) {
			printf("check_sum1:%04X  check_sum2:%04X \n",check_sum1,check_sum2);
			return -5;
		}
		return 1;	
	}
}

int finger::SendToUart(uint8_t confir,uint8_t flag, uint8_t* body, uint16_t len,uint8_t *redata)
{
	size_t pkg_len = len + 12;
	uint8_t buf[pkg_len];
	UartHead *head = (UartHead*)&buf;
	head->head = ntohs(0xEF01);
	head->addr = 0xFFFFFFFF;
	head->flag = flag;
	head->headlen = ntohs(len + 3);
	head->confir = confir;
	//校验和
	if (len > 0) {
		memcpy(&buf[sizeof(UartHead)], body, len);
	}
    *((uint16_t*)&buf[pkg_len - 2]) = ntohs(this->get_chk(&buf[6], len + 4)) ;//len+4 flag+headlen4字节
	// printf("\n-----*******send*******-------\n");
	// for (int i=0; i<pkg_len; i++) {
	// 	printf("%02X", buf[i]);
	// }
    // printf("\n");
	Send((char*)&buf, sizeof(buf));
	int relen = 0;
	int ret1 = read_pkg(redata);
	if (ret1 == 1)
	{
		UartHead *pkghead = (UartHead *)redata;
		uint8_t *body = (uint8_t *)&pkghead->confir;
		relen = ntohs(pkghead->headlen) + 9;
		// printf("\n-----*******recive*******-------\n");
		// for(int i = 0;i<relen;i++)
		// {
		// 	printf("%02X",redata[i]);
		// }
		// printf("\n");
	}
	return relen;
}
int finger::Register()//注册
{
	uint8_t conf = 0x29;
	uint8_t flag = 0x01;
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf, flag,NULL, 0, remsg);
	// for(int i = 0;i<retlen;i++)
	// {
	// 	printf("%02X",remsg[i]);
	// }
	// printf("\n");
	if (retlen > 0 && remsg[9] == 0) // 成功
	{
		return 0;
	}
	return -1;
}
int finger::ConfirFinger()//验证
{
	uint8_t conf = 0x01;
	uint8_t flag = 0x01;
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf, flag,NULL, 0, remsg);
	if (retlen > 0 && remsg[9] == 0) // 成功
	{
		return 0;
	}
	return -1;
}
int finger::GetFeature(int times)//生成特征
{
	uint8_t conf = 0x02;
	uint8_t flag = 0x01;
	uint8_t data = times;
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf,flag, &data, sizeof(data), remsg);
	if (retlen > 0 && remsg[9] == 0) // 成功
	{
		return 0;
	}
	return -1;
}
int finger::Combine()//
{
	uint8_t conf = 0x05;
	uint8_t flag = 0x01;
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf, flag,NULL, 0, remsg);
	if (retlen > 0 && remsg[9] == 0) // 成功
	{
		return 0;
	}
	return -1;
}
int finger::SaveFinger(int finger_id)//存储
{
	uint8_t conf = 0x06;
	uint8_t flag = 0x01;
	uint8_t data[3];
	uint16_t fin_id = finger_id;
	data[0] = 0x01;
	data[1] = (fin_id >> 8) & 0xFF; // 存储高位字节
	data[2] = fin_id & 0xFF;		// 存储低位字节
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf, flag, data, sizeof(data), remsg);
	printf("存指纹msgmsg------\n");
	for (int i = 0; i < retlen; i++)
	{
		printf("%02X", remsg[i]);
	}
	printf("\n");
	if (retlen > 0 && remsg[9] == 0x00) // 成功
	{
		return 0;
	}
	return -1;
}
int finger::SearchFinger(int &fingerid)//搜索
{
	uint8_t conf = 0x04;
	uint8_t flag = 0x01;
	uint8_t data[5] = {0x01,0x00,0x00,0xFF,0xFF};
	data[0] = 0x01; data[1] = 0x00; data[2] = 0x00; data[3] = 0xFF; data[4] = 0xFF;
    uint8_t remsg[20]={0};
	int retlen = SendToUart(conf, flag, data, sizeof(data), remsg);
	for (int i = 0; i < retlen; i++)
	{
		printf("%02X", remsg[i]);
	}
	printf("\n");
	if (retlen > 0 && remsg[9] == 0x00) // 成功
	{
		uint16_t finger_id = (remsg[10] >> 8) | remsg[11];
		uint16_t score = (remsg[12] >> 8) | remsg[13];
		fingerid = (int)finger_id;
		int score_s = (int)score;
		// printf("finger_id:%d  score:%d\n", fin_id, score_s);
		return 0;
	}
	return -1;
}

int finger::Delete_Finger(int finger_id)
{
	int retlen = 0;
	uint8_t remsg[20]={0};
	uint8_t conf = 0x0C; // 删除指纹
	uint8_t flag = 0x01;
	uint16_t fingers = finger_id;
	uint8_t data_[4];
	data_[0] = (fingers >> 8) & 0xFF; // 存储高位字节
	data_[1] = fingers & 0xFF;		// 存储低位字节
	data_[2] = 0x00;
	data_[3] = 0x01;
	retlen = SendToUart(conf,flag, data_, sizeof(data_), remsg);
	printf("删除msgmsg------\n");
	for (int i = 0; i < retlen; i++)
	{
		printf("%02X", remsg[i]);
	}
	printf("\n");
	if (retlen > 0 && remsg[9] == 0x00)
	{
		return 1;
	}
	return 0;
}
int finger::DeleteAllFinger()
{
	int retlen = 0;
	uint8_t remsg[20]={0};
	uint8_t conf = 0x0D; // 清空指纹
	uint8_t flag = 0x01;
	retlen = SendToUart(conf,flag, NULL, 0, remsg);
	if(retlen > 0)
	{
		printf("删除msgmsg------\n");
		for (int i = 0; i < retlen; i++)
		{
			printf("%02X", remsg[i]);
		}
		printf("\n");
		if(remsg[9] == 0x00)
		{
			return 1;
		}
	}
	return 0;
}

int finger::confir()//验证流程
{
	int finger_id = 0;
	if(ConfirFinger() == 0)//验证
	{
		if(GetFeature(1) == 0)//生成特征
		{		
			SearchFinger(finger_id);//搜索指纹
		}
	}
	return finger_id;
}

int finger::logon(int fg_id,int &fingerid)//注册流程
{
	int res;
	int times = 1;
	for(int i = 0;i < 3;i++)
	{
		time_t start_time = time(NULL);
		while(1)
		{
			time_t end_time = time(NULL);
			if(end_time - start_time >= 10)//注册超时
			{
				return -2;
			}
			if(Register() == 0)//注册
			{
				if(GetFeature(times) == 0)//生成特征
				{
					if(SearchFinger(fingerid) == 0)
					{
						return -3;
					}
					break;
				}
			}
		}
		times++;
	}
	if(Combine() == 0)//合并
	{
		if(SaveFinger(fg_id) == 0)//存储
		{
			return 0;
		}
	}
	return -1;
}

void finger::run()
{
    m_bRun = true;
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@Finger::run");
    while (m_bRun)
    {
		if(finger_status == 0)
		{
			int finger = g_finger->confir();
			g_yf->finger_opendoor(finger);
		}
		else if(finger_status == 1)
		{
			int finger_id = 0;
			printf("adduserid;%d\n",g_yf->add_userid);
			int fin_id = g_yf->g_pDB->GetMaxFingerId();
			printf("fingerid:%d\n",fin_id);
			int ret = g_finger->logon(fin_id,finger_id);
			printf("retret:%d\n",ret);
			if(ret == -3)
			{
				printf("finger is :%d\n",finger_id);
				fin_id = finger_id;
			}
			else if(ret == -2)
			{
				finger_status = 0;
			}	
			if(g_yf->add_userid > 0 && ( ret == 0 ||ret == -3))
			{
				g_yf->add_user_by_finger(g_yf->add_userid,fin_id);
				finger_status = 0;
			}
		}
		sleep(2);
        // PS_GetImage();
        // usleep(10 * 1000);
        // Process_Finger();
        // usleep(200 * 1000);
    }
    return;
}