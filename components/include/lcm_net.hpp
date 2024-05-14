#include "btn_key.hpp"

#include <lcm/lcm.h>
#include "exlcm_lcm_jyt_t.h"

struct lcm_net
{
    // std::string   src_addr;
    std::string   req_addr;
    std::string   rep_addr;
    int32_t       time_ss;
    int8_t        time_ms;
    int8_t        req_state;
    int8_t        rep_state;
    // int8_t        num_exts;
    // int8_t        *exts;

    int8_t srv_th_ms = 20;
    int is_update = false;
    int is_master = false;
    // int is_update = false;
    lcm_t *lcm = NULL;
    std::string url;
    std::string master;
    std::string ifname;
    std::string slave;
    time_t sync_now = time(NULL);
    time_t sync_old = time(NULL);
    uint16_t wait_conut = 0;
    uint16_t wait_data = 0;
    time_t wait_now = time(NULL);
    time_t wait_old = time(NULL);

    enum _lcm_state_
    {
        STATE_IDLE = 0,
        STATE_CALL,
        STATE_WORK,
        STATE_NULL,
    } lcm_state = STATE_IDLE;

    enum _lcm_role_
    {
        ROLE_NULL = 0,
        ROLE_IDLE,
        ROLE_REQER,
        ROLE_REPER,
    } lcm_role = ROLE_NULL;

    // int64_t time_state;
};

extern "C" void lcm_net_push(lcm_net *self, std::string req_addr, std::string rep_addr, 
                uint8_t req_state, uint8_t rep_state,
                int16_t num_exts = 0, int8_t *exts = NULL)
{
    exlcm_lcm_jyt_t push_data;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    push_data.time_ss = tv.tv_sec;
    push_data.time_ms = tv.tv_usec / 10000; // 10ms
    push_data.src_addr = (char *)self->slave.c_str();
    push_data.req_addr = (char *)req_addr.c_str();
    push_data.rep_addr = (char *)rep_addr.c_str();
    push_data.req_state = req_state;
    push_data.rep_state = rep_state;
    push_data.num_exts = num_exts;
    push_data.exts = exts;
    // 主机推公共池，其他人收公共池，维持同一个状态。
    exlcm_lcm_jyt_t_publish(self->lcm, (self->is_master) ? self->ifname.c_str() : self->master.c_str(), &push_data);
    self->wait_data = 0; // 假定推送同步失败
}

int lcm_net_debug = false;

void lcm_net_pull(const lcm_recv_buf_t *rbuf, const char *channel, const exlcm_lcm_jyt_t *msg, void *user)
{
    struct lcm_net *self = (struct lcm_net *)user;
    if (lcm_net_debug)
    {
        printf("self->time %d self->slave = [%s]\n", time(NULL), self->slave.c_str());
        printf("%s Received message on channel \"%s\":\n", __func__ , channel);
        printf("  time_ss       = %d\n", msg->time_ss);
        printf("  time_ms       = %d\n", msg->time_ms * 10);
        printf("  src_addr      = '%s'\n", msg->src_addr);
        printf("  req_addr      = '%s'\n", msg->req_addr);
        printf("  rep_addr      = '%s'\n", msg->rep_addr);
        printf("  req_state     = '%d'\n", msg->req_state);
        printf("  rep_state     = '%d'\n", msg->rep_state);
        printf("  num_exts      = %d\n", msg->num_exts);
        if (msg->num_exts > 0)
        {
            printf("  exts          = ");
            for (int i = 0; i < msg->num_exts; i++)
            {
                printf("%d ", msg->exts[i]);
            }
            printf("\n");
        }
    }

    self->time_ss = msg->time_ss;
    self->time_ms = msg->time_ms * 10;
    // self->src_addr = std::string(msg->src_addr);
    self->req_addr = std::string(msg->req_addr);
    self->rep_addr = std::string(msg->rep_addr);
    // 遇到消息空状态可以不更新状态。
    if (lcm_net::STATE_NULL != msg->req_state) self->req_state = msg->req_state;
    if (lcm_net::STATE_NULL != msg->rep_state) self->rep_state = msg->rep_state;

    // 快过本地的 1 秒的时间差即可同步到最新时间
    if (abs(self->time_ss - time(NULL)) > 1)
    {
        time_t t = msg->time_ss;
        stime(&t); // date -s "2019-05-29 10:58:00"
    }

    if (self->is_master && (msg->src_addr != self->slave))
    {
        self->sync_old = time(NULL);
        // 作为主机可以同步这个全局给所有设备，但要避免回环的消息
        lcm_net_push(self, self->req_addr, self->rep_addr, self->req_state, self->rep_state);
    }

    self->wait_data = 1; // 推送同步成功
}

// 使用 ifconfig 获取 dev 网卡 ip 地址并写入 /tmp/ip.cfg 后获取
std::string get_dev_ip(std::string dev)
{
    std::string ip;
    std::string cmd = string_format("ifconfig %s | grep \"inet \" | awk '{print $2}' | cut -c 6-", dev.c_str());
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp)
    {
        char buf[1024];
        fgets(buf, 1024, fp);
        pclose(fp);
        ip = buf;
    }
    if (!ip.empty() && ip[ip.length() - 1] == '\n')
        ip = ip.substr(0, ip.length() - 1);
    if (ip.length() < 7)
        ip = "";
    return ip;
}

int lcm_net_load(struct lcm_net *self, std::string ifname, std::string master, std::string url = "udpm://239.255.76.67:7667?ttl=1")
{
    self->ifname = ifname;
    system(string_format("route add -net 224.0.0.0 netmask 240.0.0.0 dev %s", self->ifname.c_str()).c_str());
    self->slave = get_dev_ip(self->ifname);
    printf("get_dev_slave = [%s]\n", self->slave.c_str());
    self->url = url;
    printf("url = [%s]\n", self->url.c_str());
    self->master = master;
    if (self->master == self->slave) self->is_master = true;
    printf("master = [%s] is_master = [%d]\n", self->master.c_str(), self->is_master);
    self->lcm = lcm_create(self->url.c_str());
    if (!self->lcm)
        return -1;
    exlcm_lcm_jyt_t_subscribe(self->lcm, (self->is_master) ? self->master.c_str() : self->ifname.c_str(), &lcm_net_pull, self);
    return 0;
}

void lcm_net_try_init(lcm_net *self)
{
    self->lcm_role = lcm_net::ROLE_NULL;
    self->lcm_state = lcm_net::STATE_IDLE;
    // 不用初始化全局变量，服务器会同步过来。
}

void lcm_net_try_reqer(lcm_net *self)
{
    // 首先确认处于待工作状态，改变状态进入工作状态
    if (self->lcm_state == lcm_net::STATE_IDLE)
    {
        self->lcm_role = lcm_net::ROLE_REQER;
        puts("lcm_net_try_reqer");
    }
}

void lcm_net_try_reper(lcm_net *self)
{
    // 首先确认处于待工作状态，改变状态进入工作状态
    if (self->lcm_state == lcm_net::STATE_IDLE)
    {
        self->lcm_role = lcm_net::ROLE_REPER;
        puts("lcm_net_try_reper");
    }
}

void lcm_net_try_exit(lcm_net *self)
{
    // 直接跳进工作状态试图等待结束
    self->lcm_state = lcm_net::STATE_WORK;
    self->lcm_role = lcm_net::ROLE_IDLE;
}

void lcm_net_idle_loop(struct lcm_net *self)
{
    // 依靠主机同步数据来刷新设备状态
    switch (self->lcm_role)
    {
    case lcm_net::ROLE_NULL:
        if (self->wait_data)
        {
            // 首先得入网，也就是收到主机应答
            // 默认状态下，主机每隔三秒会主动同步一次网络，改变从机状态。
            self->lcm_role = lcm_net::ROLE_IDLE;
            printf("%s %s into lcm_net::ROLE_IDLE\n", __func__, self->slave.c_str());
        }
        break;
    case lcm_net::ROLE_IDLE:
        if (self->wait_data)
        {
            if (self->req_state == lcm_net::STATE_CALL && self->rep_state == lcm_net::STATE_IDLE)
            {
                self->wait_now = time(NULL);
                if (self->wait_now - self->wait_old > 1)
                {
                    printf("提示有人进入请求状态，可以尝试应答\n");
                    // system("aplay /home/sound/on.wav &");
                    self->wait_old = self->wait_now;
                }
            }
        }
        break;
    case lcm_net::ROLE_REQER:
        if (self->wait_data)
        {
            if (self->req_state == lcm_net::STATE_IDLE && self->rep_state == lcm_net::STATE_IDLE)
            {
                // 试图发起请求
                lcm_net_push(self, self->slave, self->rep_addr, lcm_net::STATE_CALL, lcm_net::STATE_NULL);
                puts("试图发起请求");
            }

            // 成功同步全局变量，进入新的工作角色
            if (self->req_addr == self->slave && self->req_state == lcm_net::STATE_CALL)
            {
                self->wait_conut = 0;
                self->wait_old = time(NULL);
                self->lcm_state = lcm_net::STATE_CALL;
                printf("%s %s into lcm_net::ROLE_REQER\n", __func__, self->slave.c_str());
            }
        }
        break;
    case lcm_net::ROLE_REPER:
        if (self->wait_data)
        {
            if (self->req_state == lcm_net::STATE_CALL && self->rep_state == lcm_net::STATE_IDLE)
            {
                puts("试图应答请求");
                lcm_net_push(self, self->req_addr, self->slave, self->req_state, lcm_net::STATE_CALL);
                break;
            }

            // 成功同步全局变量，进入新的工作角色
            if (self->rep_addr == self->slave && self->rep_state == lcm_net::STATE_CALL)
            {
                self->wait_conut = 0;
                self->wait_old = time(NULL);
                self->lcm_state = lcm_net::STATE_CALL;
                printf("%s %s into lcm_net::ROLE_REPER\n", __func__, self->slave.c_str());
            }
        }
        break;
    }
}

void lcm_net_call_loop(struct lcm_net *self)
{
    // self->lcm_state = lcm_net::STATE_CALL;
    // printf("lcm_net_idle_loop\n");
    switch (self->lcm_role)
    {
    case lcm_net::ROLE_REQER:
        {
            if (self->wait_data)
            {
                // 状态同步成功后，可以建立连接了，因为应答端会根据时间对齐启动。
                if (self->req_state == lcm_net::STATE_WORK)
                {
                    // system("killall aplay");
                    printf("ROLE_REQER time = %ld\n", time(NULL));
                    system(string_format("echo '[\"tcp://%s:1030\",\"tcp://%s:2812\",\"tcp://%s:1030\",\"tcp://%s:2812\",\"work\"]' > /tmp/sample_dls.json && cd /root/bin/ && ./sample_nng &", 
                            self->rep_addr.c_str(), self->rep_addr.c_str(), self->req_addr.c_str(), self->req_addr.c_str()).c_str());
                    self->lcm_state = lcm_net::STATE_WORK; // 可以去往下一个工作状态
                    break;
                }

                self->wait_now = time(NULL);
                if (self->wait_now - self->wait_old > 1)
                {
                    self->wait_conut++;
                    self->wait_old = self->wait_now;
                    // system("aplay /home/sound/on.wav &");
                    printf("等待应答 %d\n", self->wait_conut);
                }

                if (self->wait_conut > 60) // 200ms * 60 超时了无人应答，退出复位状态。
                {
                    lcm_net_try_exit(self);
                }
                
                // 等待有人应答了回应它
                if (self->rep_state == lcm_net::STATE_CALL)
                {
                    // printf("ROLE_REQER time = %ld\n", time(NULL));
                    lcm_net_push(self, self->slave, self->rep_addr, lcm_net::STATE_WORK, self->rep_state);
                    usleep(250*1000); // 等待应答+同步状态
                    break; // rep_state 一直不变会导致到不了下面的状态
                }

            }
        }
        break;
    case lcm_net::ROLE_REPER:
            if (self->wait_data)
            {
                // 发起方回应应答方进入工作状态了，可以建立链接。
                if (self->req_state == lcm_net::STATE_WORK)
                {
                    lcm_net_push(self, self->req_addr, self->slave, self->req_state, lcm_net::STATE_WORK);
                    // system("killall aplay");
                    printf("ROLE_REPER time = %ld\n", time(NULL));
                    system(string_format("echo '[\"tcp://%s:1030\",\"tcp://%s:2812\",\"tcp://%s:1030\",\"tcp://%s:2812\",\"work\"]' > /tmp/sample_dls.json && cd /root/bin/ && ./sample_nng &", 
                            self->req_addr.c_str(), self->req_addr.c_str(), self->rep_addr.c_str(), self->rep_addr.c_str()).c_str());
                    self->lcm_state = lcm_net::STATE_WORK; // 可以去往下一个工作状态
                    break;
                }

                usleep(500*1000); // 等待应答+同步状态
                self->wait_now = time(NULL);
                if (self->wait_now - self->wait_old > 1)
                {
                    self->wait_conut++;
                    // system("aplay /home/sound/on.wav &");
                    printf("接受应答 %d\n", self->wait_conut);
                }

                if (self->wait_conut > 60) //  200ms * 60 超时了无人应答，退出复位状态。
                {
                    lcm_net_try_exit(self);
                }
            }
        break;
    }
}

void lcm_net_work_loop(struct lcm_net *self)
{
    // self->lcm_state = lcm_net::STATE_IDLE;
    // printf("lcm_net_work_loop\n");
    switch (self->lcm_role)
    {
    case lcm_net::ROLE_IDLE:
        // 在工作状态下主动改变设备回到 IDLE 状态表示自己主动挂断，一切清空，并回到原始状态。
        if (self->wait_data)
        {
            // 判断自己的与全局的角色匹配
            // if (self->req_addr == self->slave)
            // {
            //     // 有权清理自己的地址并归还状态
            //     puts("req exit");
            //     printf("req_addr = %s rep_addr = %s\n", self->req_addr.c_str(), self->rep_addr.c_str());
            //     lcm_net_push(self, "", self->rep_addr, lcm_net::STATE_IDLE, lcm_net::STATE_NULL);
            //     system("killall sample_nng");
            // }
            // else if (self->rep_addr == self->slave)
            // {
            //     puts("rep exit");
            //     printf("req_addr = %s rep_addr = %s\n", self->req_addr.c_str(), self->rep_addr.c_str());
            //     lcm_net_push(self, self->req_addr, "",lcm_net::STATE_NULL, lcm_net::STATE_IDLE);
            //     system("killall sample_nng");
            // }
            // else
            {
                // 属于其他设备，目前设计里允许停止当前网络通话，全局退出
                // puts("other exit");
                printf("req_addr = %s rep_addr = %s\n", self->req_addr.c_str(), self->rep_addr.c_str());
                lcm_net_push(self, "", "", lcm_net::STATE_IDLE, lcm_net::STATE_IDLE);
                usleep(500*1000);
                system("killall sample_nng");
            }
            if (self->rep_addr == "" && self->req_addr == "")
            {
                self->lcm_state = lcm_net::STATE_IDLE;
            }
        }
        break;
    
    case lcm_net::ROLE_REQER:
        if (self->wait_data)
        {
            // 发起方，可被改变状态退出
            if (self->req_state == lcm_net::STATE_IDLE)
            {
                self->lcm_role = lcm_net::ROLE_IDLE;
                break;
            }
        }
        break;
        
    case lcm_net::ROLE_REPER:
        if (self->wait_data)
        {
            // 发起方，可被改变状态退出
            if (self->rep_state == lcm_net::STATE_IDLE)
            {
                self->lcm_role = lcm_net::ROLE_IDLE;
                break;
            }
        }
        break;
    }
}

void lcm_net_state_loop(struct lcm_net *self)
{
    typedef void (*loop_callback_t)(struct lcm_net *self);
    const loop_callback_t loop[] = {lcm_net_idle_loop,  lcm_net_call_loop, lcm_net_work_loop};
    loop[self->lcm_state](self);
}

int lcm_net_loop(struct lcm_net *self)
{
    lcm_net_state_loop(self);
    // lcm_handle(lcm);
    lcm_handle_timeout(self->lcm, self->srv_th_ms);
    // if (get_time_ms() - last_push_time > 500)
    // {
    //     lcm_push(lcm);
    //     last_push_time = get_time_ms();
    // }
    if (self->is_master)
    {
        self->sync_now = get_time_ms();
        if (self->sync_now - self->sync_old > 250) // 每秒定时向所有设备广播一次。
        {
            lcm_net_push(self, self->req_addr, self->rep_addr, self->req_state, self->rep_state);
            self->sync_old = self->sync_now;
            self->wait_data = 1; // 主机假定立刻同步成功
        }
    }
    return 0;
}

int lcm_net_free(struct lcm_net *self)
{
    if (!self->lcm)
        lcm_destroy(self->lcm);
    return 0;
}

