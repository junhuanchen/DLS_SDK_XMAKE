#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <time.h>

// #include "XQUart.h"
// #include "finger.h"
// #include "yefiot.h"
#include "event.h"
extern finger *g_finger;
extern yf_param *g_yf;
// XQUart g_uart;
extern event *g_event;
// int state = 0; //按键状态 0 密码开门 1 长按# 2 编程密码 3功能选择
// int function = 0;//按键功能 0|添加用户

enum KeycodePassword{
	KEY_ZERO = 115, //数字1
	KEY_ONE = 114,
	KEY_TWO = 139,
	KEY_THREE = 128,
	KEY_FOUR = 28,
	KEY_FIVE = 112,
	KEY_SIX = 6,
	KEY_SEVEN = 7,
	KEY_EIGHT = 8,
	KEY_NINE = 9,
	KEY_POUND = 102,// # 号键
	// KEY_KPASTERISK = 97,// * 号键
};
struct _ts_
{
    struct timeval timeout;
    // int dev_ttyS;
    // int input_event0;
    fd_set readfd;
} ts, *self=&ts;
// 函数：将 evdev 的 keycode 转换为密码数字，如果不是我们定义的按键，返回-1

event::event()
{
    state = 0;
    input_event0 = 0;
    function = 0;
    ev_open = event_init();
    printf("ev_open:%d\n",ev_open);
}

event::~event()
{
    event_deinit();
}
bool event::event_init()
{
    input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if(input_event0 <= 0)
    {
        printf("open event fail!\n");
        return false;              
    }     
    printf("open event success!\n");
    return true;
}
void event::event_deinit()
{
	if(input_event0)
    {
        close(input_event0);
        input_event0 = 0;
    }
		
}

int event::keycode_to_password(int keycode) {
    switch (keycode) {
		case KEY_ZERO: return 0;
        case KEY_ONE: return 1;
        case KEY_TWO: return 2;
        case KEY_THREE: return 3;
        case KEY_FOUR: return 4;
		case KEY_FIVE: return 5;
		case KEY_SIX: return 6;
	    case KEY_SEVEN: return 7;
	    case KEY_EIGHT: return 8;
	    case KEY_NINE: return 9;
        case KEY_POUND: return 10;
        // case KEY_KPASTERISK: return 11;
        default: return -1;
    }
}
void event::run()
{
    m_bRun = true;
    struct input_event event;
    char password[7] = {0}; 
    int count = 0; 
    time_t last_key_time = 0; 
    int pound_key_pressed = 0; // 记录 # 号键是否被按下
    time_t pound_key_start_time = 0; // 记录 # 号键按下的开始时间
    self->timeout.tv_sec = 0;
    self->timeout.tv_usec = 0;
    printf("@@@@@@@@@@@@@@@@@@\n");
    FD_ZERO(&self->readfd);
    while(m_bRun)
    {   
        if(state != 0)
        {
            time_t current_time = time(NULL);
            if(current_time - last_key_time > 10)
            {
                count = 0;
                state = 0;
                function = 0;
                memset(password,0,sizeof(password));
            }
        }
        FD_SET(input_event0, &self->readfd);
        int ret = select(input_event0 + 1, &self->readfd, NULL, NULL, &self->timeout);
        if (ret != -1 && FD_ISSET(input_event0, &self->readfd))
        {
            // struct input_event event;		
            if (read(input_event0, &event, sizeof(event)) == sizeof(event))
            {
                if ((event.type == EV_KEY))
                {
                    // printf("keyEvent %d ->%d %s\n", event.code,event.value, (event.value) ? "Pressed" : "Released");
                    if(event.value == 1)
                    {
                        int digit = keycode_to_password(event.code);
                        printf("digit:%d\n",digit);
                        printf("state:%d\n",state);
                        if(digit != -1)
                        {
                            time_t now_time = time(NULL);
                            printf("now_time:%ld last_key_time:%ld time:%d\n",now_time,last_key_time,now_time - last_key_time);
                            if(now_time - last_key_time > 10)
                            {
                                count = 0;
                                state = 0;
                                memset(password,0,sizeof(password));
                            }
                            last_key_time = now_time;

                            if(state == 0)
                            {
                                char digitChar = digit + '0';
                                password[count] = digitChar;
                                count++;
                                if(count == 4)
                                {
                                    printf("password:%s\n",password);
                                    if(0 == strcmp(password,g_yf->opendoor_psw))
                                    {
                                        g_yf->open_door();                       
                                        // return 0;
                                    }
                                    count = 0;
                                    memset(password,0,sizeof(password));
                                }
                            }
                            else if(state == 1)
                            {
                                printf("set-psw\n");
                                char digitChar = digit + '0';
                                password[count] = digitChar;
                                count++;
                                if(count == 6)
                                {
                                    printf("setpassword:%s\n",password);
                                    if(0 == strcmp(password,g_yf->set_psw))
                                    {
                                        state = 2;
                                    }
                                    count = 0;
                                    memset(password,0,sizeof(password));
                                }
                            }
                            else if(state == 2)
                            {
                                printf("into the state\n");
                                function = digit;
                                count = 0;
                                memset(password,0,sizeof(password));
                                state = 3;
                                // return digit;
                            }
                            else if(state == 3)
                            {
                                int end = 0;
                                char digitChar = digit + '0';
                                password[count] = digitChar;
                                count++;
                                switch (function)
                                {
                                    case 1: 
                                        if(count == 3)
                                        {
                                            printf("userid:%s\n",password);
                                            g_yf->add_userid = atoi(password);
                                            g_finger->finger_status = 1;
                                            sleep(1);  
                                            end = 1;                                        
                                        }
                                        break;
                                    case 2: 
                                        if(count == 4)
                                        {
                                            printf("open_psw:%s\n",password);
                                            g_yf->updata_opendoor_psw(password);
                                            end = 1;
                                        }
                                        break;
                                    case 3: 
                                        if(count == 3)
                                        {
                                            printf("userid:%s\n",password);
                                            int user_id = atoi(password);                          
                                            g_yf->del_user(user_id);
                                            end = 1;
                                        }
                                        break;
                                    case 4: //删除全部用户
                                        if(count == 6)
                                        {
                                            printf("setpassword:%s\n",password);
                                            if(0 == strcmp(password,g_yf->set_psw))
                                            {
                                                g_yf->del_user();
                                            }
                                            end = 1;
                                        }
                                        break;
                                    case 5: //修改开门延时
                                        if(count == 2)
                                        {
                                            printf("open_delay:%s\n",password);
                                            int open_delay = atoi(password);
                                            g_yf->updata_open_delay(open_delay);
                                            end = 1;
                                        }
                                        break;
                                    case 6: //初始化
                                        if(count == 6)
                                        {
                                            printf("setpassword:%s\n",password);
                                            if(0 == strcmp(password,g_yf->set_psw))
                                            {
                                                g_yf->restore_init();
                                            }
                                            end = 1;
                                        }
                                        break;
                                    case 7: //设置增加卡
                                        break;
                                    case 8: //设置删除卡
                                        break;
                                    case 9: //修改编程密码
                                        if(count == 6)
                                        {
                                            printf("newsetpassword:%s\n",password);
                                            g_yf->updata_set_psw(password);
                                            end = 1;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                if(end == 1)
                                {
                                    count = 0;
                                    memset(password,0,sizeof(password));
                                }
                            }
                            if (digit == 10) {
                                pound_key_pressed = 1;
                                pound_key_start_time = now_time;
                            }
                        }
                    }
                    else if (event.value == 0)
                    {
                        if (keycode_to_password(event.code) == 10 && pound_key_pressed) 
                        {
                            if(state != 0)
                            {
                                state = 0;
                                if(function == 1)
                                    g_finger->finger_status = 0;
                            }
                            else
                            {
                                time_t current_time = time(NULL);
                                printf("key_time:%d\n",current_time - pound_key_start_time);
                                if (current_time - pound_key_start_time >= 1) 
                                { // 长按一秒
                                    state = 1; // 进入输入设置密码状态
                                }
                            }
                            count = 0;
                            function = 0;
                            memset(password,0,sizeof(password));
                            pound_key_pressed = 0; // 重置 # 号键状态
                        }
                    }
                }
            }
        }
    }
}





// int event_init()
// {
//     self->input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
//     if (self->input_event0 <= 0)
//     {
//       perror("open /dev/input/event0 device error!\n");
//       abort();
//     }
//     self->timeout.tv_sec = 0;
//     self->timeout.tv_usec = 0;
    
//     FD_ZERO(&self->readfd);
// }
// int event_deinit()
// {
// 	if(self->input_event0)
// 		close(self->input_event0);
// }

// int linux_event_main() {
//     puts("eventbegin!");
//     // 打开设备文件
//     event_init();
//     struct input_event event;
    
//     char password[7] = {0}; 
//     int count = 0; 
//     time_t last_key_time = 0; 
    
//     int pound_key_pressed = 0; // 记录 # 号键是否被按下
//     time_t pound_key_start_time = 0; // 记录 # 号键按下的开始时间
//     while(1)
//     {
//         if(state != 0)
//         {
//             time_t current_time = time(NULL);
//             if(current_time - last_key_time > 10)
//             {
//                 count = 0;
//                 state = 0;
//                 function = 0;
//                 memset(password,0,sizeof(password));
//             }
//         }
//         FD_SET(self->input_event0, &self->readfd);
//         int ret = select(self->input_event0 + 1, &self->readfd, NULL, NULL, &self->timeout);
//         if (ret != -1 && FD_ISSET(self->input_event0, &self->readfd))
//         {
//             // struct input_event event;		
//             if (read(self->input_event0, &event, sizeof(event)) == sizeof(event))
//             {
//                 if ((event.type == EV_KEY))
//                 {
//                     // printf("keyEvent %d ->%d %s\n", event.code,event.value, (event.value) ? "Pressed" : "Released");
//                     if(event.value == 1)
//                     {
//                         int digit = g_event->keycode_to_password(event.code);
//                         printf("digit:%d\n",digit);
//                         printf("state:%d\n",state);
//                         if(digit != -1)
//                         {
//                             time_t now_time = time(NULL);
//                             printf("now_time:%ld last_key_time:%ld time:%d\n",now_time,last_key_time,now_time - last_key_time);
//                             if(now_time - last_key_time > 10)
//                             {
//                                 count = 0;
//                                 state = 0;
//                                 memset(password,0,sizeof(password));
//                             }
//                             last_key_time = now_time;

//                             if(state == 0)
//                             {
//                                 char digitChar = digit + '0';
//                                 password[count] = digitChar;
//                                 count++;
//                                 if(count == 4)
//                                 {
//                                     printf("password:%s\n",password);
//                                     if(0 == strcmp(password,g_yf->opendoor_psw))
//                                     {
//                                         g_yf->open_door();                       
//                                         // return 0;
//                                     }
//                                     count = 0;
//                                     memset(password,0,sizeof(password));
//                                 }
//                             }
//                             else if(state == 1)
//                             {
//                                 printf("set-psw\n");
//                                 char digitChar = digit + '0';
//                                 password[count] = digitChar;
//                                 count++;
//                                 if(count == 6)
//                                 {
//                                     printf("setpassword:%s\n",password);
//                                     if(0 == strcmp(password,g_yf->set_psw))
//                                     {
//                                         state = 2;
//                                     }
//                                     count = 0;
//                                     memset(password,0,sizeof(password));
//                                 }
//                             }
//                             else if(state == 2)
//                             {
//                                 printf("into the state\n");
//                                 function = digit;
//                                 count = 0;
//                                 memset(password,0,sizeof(password));
//                                 state = 3;
//                                 // return digit;
//                             }
//                             else if(state == 3)
//                             {
//                                 int end = 0;
//                                 char digitChar = digit + '0';
//                                 password[count] = digitChar;
//                                 count++;
//                                 switch (function)
//                                 {
//                                     case 1: 
//                                         if(count == 3)
//                                         {
//                                             printf("userid:%s\n",password);
//                                             add_userid = atoi(password);
//                                             finger_staus = 1;
//                                             sleep(1);  
//                                             end = 1;                                        
//                                         }
//                                         break;
//                                     case 2: 
//                                         if(count == 4)
//                                         {
//                                             printf("open_psw:%s\n",password);
//                                             g_yf->updata_opendoor_psw(password);
//                                             end = 1;
//                                         }
//                                         break;
//                                     case 3: 
//                                         if(count == 3)
//                                         {
//                                             printf("userid:%s\n",password);
//                                             int user_id = atoi(password);                          
//                                             g_yf->del_user(user_id);
//                                             end = 1;
//                                         }
//                                         break;
//                                     case 4: //删除全部用户
//                                         if(count == 6)
//                                         {
//                                             printf("setpassword:%s\n",password);
//                                             if(0 == strcmp(password,g_yf->set_psw))
//                                             {
//                                                 g_yf->del_user();
//                                             }
//                                             end = 1;
//                                         }
//                                         break;
//                                     case 5: //修改开门延时
//                                         if(count == 2)
//                                         {
//                                             printf("open_delay:%s\n",password);
//                                             int open_delay = atoi(password);
//                                             g_yf->updata_open_delay(open_delay);
//                                             end = 1;
//                                         }
//                                         break;
//                                     case 6: //初始化
//                                         if(count == 6)
//                                         {
//                                             printf("setpassword:%s\n",password);
//                                             if(0 == strcmp(password,g_yf->set_psw))
//                                             {
//                                                 g_yf->restore_init();
//                                             }
//                                             end = 1;
//                                         }
//                                         break;
//                                     case 7: //设置增加卡
//                                         break;
//                                     case 8: //设置删除卡
//                                         break;
//                                     case 9: //修改编程密码
//                                         if(count == 6)
//                                         {
//                                             printf("newsetpassword:%s\n",password);
//                                             g_yf->updata_set_psw(password);
//                                             end = 1;
//                                         }
//                                         break;
//                                     default:
//                                         break;
//                                 }
//                                 if(end == 1)
//                                 {
//                                     count = 0;
//                                     memset(password,0,sizeof(password));
//                                 }
//                             }
//                             if (digit == 10) {
//                                 pound_key_pressed = 1;
//                                 pound_key_start_time = now_time;
//                             }
//                         }
//                     }
//                     else if (event.value == 0)
//                     {
//                         if (g_event->keycode_to_password(event.code) == 10 && pound_key_pressed) 
//                         {
//                             if(state != 0)
//                             {
//                                 state = 0;
//                             }
//                             else
//                             {
//                                 time_t current_time = time(NULL);
//                                 printf("key_time:%d\n",current_time - pound_key_start_time);
//                                 if (current_time - pound_key_start_time >= 1) 
//                                 { // 长按一秒
//                                     state = 1; // 进入输入设置密码状态
//                                 }
//                             }
//                             count = 0;
//                             function = 0;
//                             memset(password,0,sizeof(password));
//                             pound_key_pressed = 0; // 重置 # 号键状态
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     return 0;
// }