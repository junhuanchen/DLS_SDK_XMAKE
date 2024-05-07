
#include "lcm_net.hpp"

struct _main_data_
{
    int exit;
    struct lcm_net lcm;
    struct btn_key key;
} main_data, *main_self = &main_data;

#include <signal.h>
// ctrl + c 停止程序
void signalHandler(int signum)
{
    printf("Caught signal %d\n", signum);
    main_self->exit = 1;
}

// echo '["wlan0","udpm://239.255.76.67:7667?ttl=1","192.168.137.192","work"]' > /tmp/demo.json && ./demo
int main(int argc, char **argv)
{
    std::string ifname = "lo";
    std::string url = "udpm://239.255.76.67:7667?ttl=1";
    std::string host = "127.0.0.1";
    std::string user_data = "127.0.0.1";

    if (access("/tmp/demo.json", F_OK) == 0)
    {
      printf("/tmp/demo.json exist!\n");

      auto conf = string_read_file("/tmp/demo.json");

      try
      {
        auto cfg = json5pp::parse(conf);
        ifname = cfg.as_array().at(0).as_string();
        url = cfg.as_array().at(1).as_string();
        host = cfg.as_array().at(2).as_string();
        user_data = cfg.as_array().at(3).as_string();
        if (user_data == "debug")
        {
            extern int lcm_net_debug;
            lcm_net_debug = true;
        }
        printf("user_data: %s\n", user_data.c_str());
      }
      catch (json5pp::syntax_error e)
      {
        printf("load_json_conf %s : %s", conf.c_str(), e.what());
      }
      
      system("rm /tmp/demo.json");
    }

    // 注册信号处理程序
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGQUIT, signalHandler);
    
    btn_key_load(&main_self->key);
    lcm_net_load(&main_self->lcm, ifname, host, url);
    while (!main_self->exit)
    {
        lcm_net_loop(&main_self->lcm);
        uint16_t code = 0, value = 0;
        if (btn_key_loop(&main_self->key, &code, &value))
        {
            // printf("code = %d, value = %d\n", code, value);
            if (value == 0)
            {
                switch (code)
                {
                case 101:
                    printf("101\n");
                    lcm_net_try_reqer(&main_self->lcm);
                    break;
                case 102:
                    printf("102\n");
                    lcm_net_try_reper(&main_self->lcm);
                    break;
                case 104:
                    printf("104\n");
                    lcm_net_try_exit(&main_self->lcm);
                    break;
                case 105:
                    printf("105\n");
                    lcm_net_try_init(&main_self->lcm);
                    break;
                }
            }
        }
    }
    lcm_net_free(&main_self->lcm);
    btn_key_free(&main_self->key);
    
    return 0;
}

