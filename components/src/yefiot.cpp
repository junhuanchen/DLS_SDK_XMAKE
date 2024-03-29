#include <stdlib.h>
#include "yefiot.h"
#include "stdint.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <time.h>
using namespace std;

#include "finger.h"
#include "common.h"
extern yf_param *g_yf;
finger *g_finger = NULL;
event *g_event = NULL;

static void *finger_pthread(void *arg)
{
    pthread_detach(pthread_self());
    printf("finger thread=======\n");
    yf_param * g_yf = (yf_param *)arg;
    if(g_yf)
    {
        printf("finger-----------\n");
        g_yf->finger_uart_main();
    }
    pthread_exit((void *)1);
    return NULL;     
}
static void *event_pthread(void *arg)
{ 
    pthread_detach(pthread_self());
    yf_param * g_yf = (yf_param *)arg;
    if(g_yf)
    {
        g_yf->linux_event_main(); 
    }
    pthread_exit((void *)1);
    return NULL;    
}

yf_param::yf_param()
{
	g_pDB = new XQDB();
    pthread_t t1,t2;
    pthread_create(&t1,NULL,finger_pthread,this);
    pthread_create(&t2,NULL,event_pthread,this);
}
yf_param::~yf_param()
{
    SAFE_RELEASE(g_pDB);
    SAFE_RELEASE(g_finger);
    SAFE_RELEASE(g_event);
    // SAFE_RELEASE(m_pTimeSec_Gloabl);
    // SAFE_RELEASE(m_pFinger);
}

bool fileExists(const std::string& filename) //yefiot
{
std::ifstream file(filename);
return file.good(); // 检查文件流的状态
}
void yf_param::setparameter(char *data)
{
    cJSON * cj_root = cJSON_Parse(data);          
    if (cj_root)
    {
        cJSON* open_delay = cJSON_GetObjectItem(cj_root,"open_delay");
        cJSON *opendoor_psw = cJSON_GetObjectItem(cj_root, "opendoor_psw");
        cJSON *set_psw = cJSON_GetObjectItem(cj_root, "set_psw");
        if(open_delay)
            g_yf->open_delay = atoi(open_delay->valuestring);
        if(opendoor_psw)
            sprintf(g_yf->opendoor_psw, "%s", opendoor_psw->valuestring);
            printf("opendoor_psw:%s\n",opendoor_psw->valuestring);
        if(set_psw)
            sprintf(g_yf->set_psw, "%s", set_psw->valuestring);
            printf("set_psw:%s\n",set_psw->valuestring);
    }
    cJSON_Delete(cj_root);
}
void yf_param::getparameter()
{
    std::string filename = "/data/param.txt";
    if(fileExists(filename))//文件存在
    {
        std::ifstream file(filename);
        char data[80]={0};
        if(file.is_open())
        {
            file.read(data,sizeof(data));
            file.close();
        }
        printf("readdata:%s\n",data);
        setparameter(data);     
    }
    else //不存在,创建
    {
        printf("File not exit");
        char data[80]={0};
        sprintf(data,"{\"open_delay\":\"3\",\"opendoor_psw\":\"1111\",\"set_psw\":\"000000\"}");
        std::ofstream file(filename);
        if(file.is_open())
        {
            std::string door_data=data;
            file.write(door_data.c_str(),door_data.size());
            file.close();
        }  
        printf("data:%s\n",data);
        setparameter(data); 
    }
}
int yf_param::updata_opendoor_psw(char *opendoor_psw)
{
    char data[80]={0};
	std::fstream fint("/data/param.txt", std::ios::in);
	if (!fint.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fint.read(data, sizeof(data));
	fint.close();
	cJSON * cj_root = cJSON_Parse(data);
	cJSON_ReplaceItemInObject(cj_root, "opendoor_psw", cJSON_CreateString(opendoor_psw));

	char *door_data = cJSON_Print(cj_root);
	std::fstream fout("/data/param.txt", std::ios::out);
	if (!fout.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fout.write(door_data, strlen(door_data));
	fout.close();
    setparameter(door_data);
	// free(door_data);
	return 1;
}
int yf_param::updata_set_psw(char *set_psw)
{
    char data[80]={0};
	std::fstream fint("/data/param.txt", std::ios::in);
	if (!fint.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fint.read(data, sizeof(data));
	fint.close();
	cJSON * cj_root = cJSON_Parse(data);
	cJSON_ReplaceItemInObject(cj_root, "set_psw", cJSON_CreateString(set_psw));

	char *door_data = cJSON_Print(cj_root);
	std::fstream fout("/data/param.txt", std::ios::out);
	if (!fout.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fout.write(door_data, strlen(door_data));
	fout.close();
    setparameter(door_data);
	free(door_data);
	return 1;
}
int yf_param::updata_open_delay(int open_delay)
{
    char data[80]={0};
	std::fstream fint("/data/param.txt", std::ios::in);
	if (!fint.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fint.read(data, sizeof(data));
	fint.close();
	cJSON * cj_root = cJSON_Parse(data);
	cJSON_ReplaceItemInObject(cj_root, "open_delay", cJSON_CreateString((const char*)open_delay));

	char *door_data = cJSON_Print(cj_root);
	std::fstream fout("/data/param.txt", std::ios::out);
	if (!fout.is_open())
	{
		printf("open file fail\n");
		return 0;
	}
	fout.write(door_data, strlen(door_data));
	fout.close();
    setparameter(door_data);
	free(door_data);
	return 1;
}
int yf_param::open_door()
{
    system("aplay /home/res/audio/startup.wav");
    return 0;
}
int yf_param::add_user(int user_id)
{
    int ret = 0;
    g_finger->finger_status = 1;
    sleep(1);
    // pthread_mutex_lock(&mutex);
    // while(finger_g ==0)
    // {
        // pthread_cond_wait(&cond, &mutex); // 等待条件变量
    // }
    // printf("Received finger_g value: %d\n", finger_g);
    // pthread_mutex_unlock(&mutex);
    
    
    return ret;
}
int yf_param::del_user(int user_id)
{
    int ret = 0;
    
    g_yf->g_pDB->OpenDB();
    if(user_id == -1)
    {
        printf("delete all finger\n");
        g_finger->finger_status = 2;
        sleep(1);
        g_finger->DeleteAllFinger();
        ret = g_yf->g_pDB->DeleteAllUser();
    }
    else
    {
        int finger_id = 0;
        g_pDB->GetFingerByUserId(user_id,finger_id);
        printf("delete fingerid;%d\n",finger_id);
        g_finger->finger_status = 2;
        sleep(1);
        g_finger->Delete_Finger(finger_id);
        ret = g_yf->g_pDB->DeleteUserByUserid(user_id);
    }
    g_yf->g_pDB->CloseDB();
    g_finger->finger_status = 0;
    return ret;
}
void yf_param::restore_init()
{
    system("rm -f /data/user.db");
	system("rm -f /data/record.db");
    remove("/data/param.txt");
    g_finger->finger_status = 2;
    sleep(1);
    g_yf->g_pDB->DeleteAllUser();
    system("reboot");
}
void yf_param::add_record(int user_id,int access_type,int record_time)
{
    std::list<user_data> p_user_data;
    p_user_data.clear();
    g_yf->g_pDB->GetItemByUserId(user_id,p_user_data);
    if(p_user_data.size())
    {
        const char *card;
        int finger_id;
        for (const auto &cell : p_user_data)
        {
            card = cell.card_num.c_str();
            finger_id = cell.finger_id;
        }
        char insertdata[180] = {0};
        sprintf(insertdata, "INSERT INTO t_record_data(record_time,acess_type,user_card,user_id) VALUES(%d,%d,'%s',%d);", record_time, access_type, card, user_id);
        g_yf->g_pDB->insertData(insertdata);
    }
}
int yf_param::finger_uart_main()
{
    g_finger = new finger;
    if(g_finger)
    {
        g_finger->start();

    }
}
int yf_param::linux_event_main()
{
    g_event = new event;
    printf("envet new finger\n");
    if(g_event)
    {
        g_event->start();
    }
}
int yf_param::finger_opendoor(int fingerid)
{
    g_pDB->OpenDB();
    if(fingerid > 0) 
    {
        printf("finger:%d\n",fingerid);
        int user_id;			
        g_pDB->GetUseridByFingerId(fingerid,user_id);
        if(user_id > 0)
        {
            open_door();
            time_t setTime;
            time(&setTime);
            add_record(user_id,1,setTime);
        }
        
    }
    g_pDB->CloseDB();
}
int yf_param::add_user_by_finger(int userid,int fingerid)
{
    int ret = 0;
    std::list<user_data> p_user_data;
    p_user_data.clear();
    g_yf->g_pDB->OpenDB();
    g_yf->g_pDB->GetItemByUserId(userid,p_user_data);
    if(p_user_data.size())
    {
        int f_id;
        for (const auto &cell : p_user_data)
        {
            f_id = cell.finger_id;
        }
        if(f_id != 0 && f_id != fingerid)
        {
            int del_success = g_finger->Delete_Finger(f_id);
            ret = g_yf->g_pDB->UpdateFinger(userid,fingerid);
        }
    }
    else
    {
        char data[80]={0};
        sprintf(data, "INSERT INTO tb_user_data(user_id,card_num,finger_id) VALUES(%d,' ',%d);", userid, fingerid);
        int ret = g_yf->g_pDB->insertData(data);
    }
    g_yf->g_pDB->CloseDB();
    return ret;
}
// int yf_param::(int user_id)
// {
//     g_yf->g_pDB->GetItemByFingerId(finger_id,p_user_data);
//     if(p_user_data.size())
//     {
//         g_yf->open_door();
//         const char *card;
//         int user_id;
//         for (const auto &cell : p_user_data)
//         {
//             user_id = cell.user_id;
//             card = cell.card_num.c_str();
//         }
//     }
// }
