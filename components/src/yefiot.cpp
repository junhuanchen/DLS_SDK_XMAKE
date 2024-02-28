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

// extern yf_param *g_yf_param;

// yf_param::yf_param()
// {
// 	this->open_delay = 3; // 
//     snprintf(this->opendoor_psw, 5, "7890");
//     snprintf(this->set_psw, 7, "123456");
// }
int handle_key_button(int mode)
{
    if(mode == 1)//添加用户
    {}
    else if(mode == 2)//修改开门密码
    {}
    else if(mode == 3)//删除用户
    {}
    else if(mode == 4)//删除全部用户
    {}
    else if(mode == 5)//修改开门延时
    {}
    else if(mode == 6)//恢复出厂
    {}
    else if(mode == 7)//设置增加卡
    {}
    else if(mode == 8)//设置删除卡
    {}
    else if(mode == 9)//修改变成密码
    {}
    return 0;
}