
#include "sample_common.h"

#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <sstream>
#include <list>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include "json5pp.hpp"

#define TCP_NODELAY		1	/* Turn off Nagle's algorithm. */
#define TCP_CORK		3	/* Never send partially complete segments */
#define TCP_QUICKACK		12	/* Block/reenable quick acks */

#define PRINT_LOG // PRINT_LOG

#define CALC_FPS(tips)                                                                                         \
    {                                                                                                          \
        static int fcnt = 0;                                                                                   \
        fcnt++;                                                                                                \
        static struct timespec ts1, ts2;                                                                       \
        clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
        if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
        {                                                                                                      \
            printf("%s => FPS:%d     \r\n", tips, fcnt);                                                  \
            ts1 = ts2;                                                                                         \
            fcnt = 0;                                                                                          \
        }                                                                                                      \
    }

// << string_format("%d", 202412);
template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args...); // Extra space for \0
    // unique_ptr<char[]> buf(new char[size]);
    char bytes[size];
    snprintf(bytes, size, format.c_str(), args...);
    return std::string(bytes);
}

void string_write_file(std::string path, std::string txt)
{
    std::ofstream outfile(path);
    outfile << txt;
    outfile.flush();
    outfile.close();
}

static int64_t get_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

std::string string_read_file(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

struct _sample_cfg_
{
    bool is_load = false;
    bool has_cfg = false;
    int capture = false;

    std::string audio_recv_ip;
    std::string audio_recv_port;

    std::string audio_send_ip;
    std::string audio_send_port;

    std::string video_recv_ip;
    std::string video_recv_port;

    std::string video_send_ip;
    std::string video_send_port;

    void get_addr_port(std::string addr, std::string &ip, std::string &port)
    {
        std::string::size_type pos = addr.find_last_of(':');
        ip = addr.substr(0, pos);
        port = addr.substr(pos + 1);
        printf("ip [%s] port [%s]\n", ip.c_str(), port.c_str());
    }

    void sample_cfg_load(std::string audio_send_addr, std::string video_send_addr, std::string audio_recv_addr, std::string video_recv_addr)
    {
        sample_cfg_exit();
        if (is_load == false)
        {
            get_addr_port(audio_send_addr, audio_send_ip, audio_send_port);
            get_addr_port(video_send_addr, video_send_ip, video_send_port);
            get_addr_port(audio_recv_addr, audio_recv_ip, audio_recv_port);
            get_addr_port(video_recv_addr, video_recv_ip, video_recv_port);
            has_cfg = true;
            is_load = true;
        }
    }

    void sample_cfg_exit()
    {
        if (is_load)
        {
            is_load = false;
        }
    }

    void sample_cfg_loop()
    {
        if (access("/tmp/sample_cfg.json", F_OK) == 0)
        {
            // echo '["0.0.0.0:1031","0.0.0.0:2813","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","exit"]' > /tmp/sample_cfg.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","exit"]' > /tmp/sample_cfg.json
            auto conf = string_read_file("/tmp/sample_cfg.json");

            try
            {
                auto cfg = json5pp::parse(conf);
                auto audio_send_addr = cfg.as_array().at(0).as_string();
                auto video_send_addr = cfg.as_array().at(1).as_string();
                auto audio_recv_addr = cfg.as_array().at(2).as_string();
                auto video_recv_addr = cfg.as_array().at(3).as_string();
                auto user_data = cfg.as_array().at(4).as_string();

                PRINT_LOG("user_data: %s\n", user_data.c_str());
                if (user_data == "exit")
                {
                    sample_cfg_exit();
                    sample_nng_signalHandler(0);
                }
                else if (user_data == "load")
                {
                    sample_cfg_load(audio_send_addr, video_send_addr, audio_recv_addr, video_recv_addr);
                }
                else if (user_data == "capture")
                {
                    capture = 3; // 抓拍次数
                }
            }
            catch (json5pp::syntax_error e)
            {
                PRINT_LOG("load_json_conf %s : %s", conf.c_str(), e.what());
            }

            system("rm /tmp/sample_cfg.json");
        }

        system("echo 3 > /proc/sys/vm/drop_caches");

        if (is_load == false) // 如果上电没有配置项，为了初始化，自己生成。
        {
            system("echo '[\"0.0.0.0:1030\",\"0.0.0.0:2814\",\"0.0.0.0:1030\",\"0.0.0.0:2814\",\"load\"]' > /tmp/sample_cfg.json");
            is_load = true;
        }
    }

} sample_cfg, *cfgs = &sample_cfg;

#include "TCPServer.h"
#include "TCPClient.h"

#include <nngpp/nngpp.h>
#include <nngpp/platform/platform.h>
#include <nngpp/protocol/pull0.h>
#include <nngpp/protocol/push0.h>

extern "C"
{
    extern void sample_nng_signalHandler(int signum);

    auto audioLogPrinter = [](const std::string &strLogMsg)
    {
        return;
        std::cout << "[audio]" << strLogMsg << std::endl; 
    };

    auto videoLogPrinter = [](const std::string &strLogMsg)
    {
        return; 
        std::cout << "[video]" << strLogMsg << std::endl; 
    };

    struct _sample_socket_
    {
        CTCPClient *audio_send = nullptr;
        CTCPServer *audio_recv = nullptr;
        ASocket::Socket audio_ConnectedClient = INVALID_SOCKET;

        nng::socket *video_send = nullptr;//video_send;// = nng::push::open();
        nng::socket *video_recv = nullptr;//video_recv;// = nng::pull::open();

        uint16_t DLS_VO = 0;
        uint16_t DLS_VI = 0;

        uint8_t audio_send_state = 0;
        uint8_t audio_recv_state = 0;
        uint8_t video_send_state = 0;
        uint8_t video_recv_state = 0;

        int32_t sample_sock_sum = 0;
        int64_t audio_send_old = 0;
        int64_t audio_recv_old = 0;
        int64_t video_send_old = 0;
        int64_t video_recv_old = 0;
        int64_t sample_sock_new = 0;
        
        _sample_socket_()
        {
            sample_sock_new = get_time_ms();
            audio_send_old = sample_sock_new;
            audio_recv_old = sample_sock_new;
            video_send_old = sample_sock_new;
            video_recv_old = sample_sock_new;
        }

        void sample_socket_loop()
        {
            // 将链接状态写入到 /tmp/sample_socket
            string_write_file("/tmp/sample_socket", string_format("[%04hx,%04hx,%04hx,%04hx]", audio_send_state, audio_recv_state, video_send_state, video_recv_state));
            // 可检查工作状态，如果状态从 1 到 0 的过程中时间超过了 3 秒，可以认为链接断开，主动退出程序，并期望管理程序重新创建对讲进程。
            return ; // 备用超时逻辑，用于主动退出对讲无法连接或异常连接的情况。

            sample_sock_new = get_time_ms();
            if (audio_send_state == 1)
            {
                audio_send_old = sample_sock_new;
                sample_sock_sum = 0;
            }
            if (audio_recv_state == 1)
            {
                audio_recv_old = sample_sock_new;
                sample_sock_sum = 0;
            }
            if (DLS_VI && video_send_state == 1)
            {
                video_send_old = sample_sock_new;
                sample_sock_sum = 0;
            }
            if (DLS_VO && video_recv_state == 1)
            {
                video_recv_old = sample_sock_new;
                sample_sock_sum = 0;
            }
            if ((sample_sock_new - audio_send_old) > 2000)
            {
                PRINT_LOG("audio_send_state timeout = %d\n", audio_send_state);
                audio_send_old = sample_sock_new;
                sample_sock_sum += 1;
            }
            if ((sample_sock_new - audio_recv_old) > 2000)
            {
                PRINT_LOG("audio_recv_state timeout = %d\n", audio_recv_state);
                audio_recv_old = sample_sock_new;
                sample_sock_sum += 1;
            }
            if (DLS_VI && (sample_sock_new - video_send_old) > 2000)
            {
                PRINT_LOG("video_send_state timeout = %d\n", video_send_state);
                video_send_old = sample_sock_new;
                sample_sock_sum += 1;
            }
            if (DLS_VO && (sample_sock_new - video_recv_old) > 2000)
            {
                PRINT_LOG("video_recv_state timeout = %d\n", video_recv_state);
                video_recv_old = sample_sock_new;
                sample_sock_sum += 1;
            }
            if (sample_sock_sum > 5) // 超时的任务过多，并且没有修正过来，可以选择退出
            {
                printf("sample_sock_sum timeout = %d > 5 \n", sample_sock_sum);
                sample_nng_signalHandler(0);
            }
        }

        ~_sample_socket_()
        {
            if (audio_send)
            {
                auto bak = audio_send;
                audio_send = NULL;
                usleep(40*1000);
                delete audio_send;
            }

            if (audio_recv)
            {
                auto bak = audio_recv;
                audio_recv = NULL;
                usleep(40*1000);
                delete audio_recv;
            }

            if (audio_ConnectedClient != INVALID_SOCKET)
            {
                audio_ConnectedClient = INVALID_SOCKET;
            }

            if (video_send)
            {
                auto bak = video_send;
                video_send = NULL;
                usleep(40*1000);
                delete video_send;
            }
            
            if (video_recv)
            {
                auto bak = video_recv;
                video_recv = NULL;
                usleep(40*1000);
                delete video_recv;
            }
        }
    } sample_socket, *sockets = &sample_socket;

    static int sample_sock_audio_send(uint8_t *data, int size)
    {
        sockets->audio_send_state = 0;

        if (!cfgs->has_cfg) { usleep(40*1000); return -1; }
        
        PRINT_LOG("sample_sock_audio_send size = %d\n", size);
        if (sockets->audio_send == nullptr)
        {
            sockets->audio_send = new CTCPClient(audioLogPrinter);
        }
        if (!sockets->audio_send->IsConnected())
        {
            if (!sockets->audio_send->Connect(cfgs->audio_send_ip, cfgs->audio_send_port))
            {
                PRINT_LOG("Connect %s:%s failed\n", cfgs->audio_send_ip.c_str(), cfgs->audio_send_port.c_str());        
                sockets->audio_send_state = 0;
                return -1;
            }
            sockets->audio_send->SetSndTimeout(40);
            PRINT_LOG("Connect %s:%s success\n", cfgs->audio_send_ip.c_str(), cfgs->audio_send_port.c_str());
        }
        int ret = sockets->audio_send->Send((const char*)data, size);
        if (!ret)
        {
            PRINT_LOG("Send %s:%s failed\n", cfgs->audio_send_ip.c_str(), cfgs->audio_send_port.c_str());
            sockets->audio_send_state = 0;
            return -1;
        }
        PRINT_LOG("Send success %s:%s %d\n", cfgs->audio_send_ip.c_str(), cfgs->audio_send_port.c_str(), ret);
        
        // {
        //     static int cnt = 0;
        //     if (++cnt > 100)
        //     {
        //         cnt = 0;
        //         sockets->audio_send->Disconnect();
        //         puts("reconnect sample_sock_audio_send");
        //     }
        // }
        
        sockets->audio_send_state = 1;

        return size;
    }

    static int sample_sock_audio_recv(uint8_t *data, int size)
    {
        if (!cfgs->has_cfg) { usleep(40*1000); return -1; }
        
        PRINT_LOG("sample_sock_audio_recv size = %d\n", size);
        if (sockets->audio_recv == nullptr)
        {
            sockets->audio_recv = new CTCPServer(audioLogPrinter, cfgs->audio_recv_port);
        }

        if(sockets->audio_ConnectedClient == INVALID_SOCKET)
        {
            int ret = sockets->audio_recv->Listen(sockets->audio_ConnectedClient, 40);
            if (!ret)
            {
                PRINT_LOG("Listen %s failed\n", cfgs->audio_recv_port.c_str());
                sockets->audio_recv_state = 0;
                return -1;
            }
            // sockets->audio_recv->SetRcvTimeout(sockets->audio_ConnectedClient, 40);
            PRINT_LOG("Listen %s success\n", cfgs->audio_recv_port.c_str());
        }

        int ret = ASocket::SelectSocket(sockets->audio_ConnectedClient, 40);
        if (ret > 0)
        {
            int read_size = sockets->audio_recv->Receive(sockets->audio_ConnectedClient, (char *)data, size, false);
            if (read_size <= 0)
            {
                PRINT_LOG("Receive %s failed\n", cfgs->audio_recv_port.c_str());
                sockets->audio_ConnectedClient = INVALID_SOCKET;
                // delete sockets->audio_recv;
                // sockets->audio_recv = nullptr;
                sockets->audio_recv_state = 0;
                return -1;
            }
            size = read_size;
            PRINT_LOG("Receive %s success %d\n", cfgs->audio_recv_port.c_str(), read_size);
        }
        
        // {
        //     static int cnt = 0;
        //     if (++cnt > 40)
        //     {
        //         cnt = 0;
        //         sockets->audio_recv->Disconnect(sockets->audio_ConnectedClient);
        //     }
        // }

        sockets->audio_recv_state = 1;

        return size;
    }

    static int sample_sock_video_send(uint8_t *data, int size)
    {
        try
        {
            if (!cfgs->has_cfg) { usleep(40*1000); return -1; }
            
            if (cfgs->capture > 0)
            {
                PRINT_LOG("sample_sock_video_send capture\n");
                string_write_file(string_format("./captur_%d.jpg", cfgs->capture), std::string((char *)data, size));
                cfgs->capture -= 1;
            }

            if (!sockets->video_send)
            {
                sockets->video_send = new nng::socket();
                *sockets->video_send = nng::push::open();
                nng::set_opt_send_timeout(*sockets->video_send, 40);
                nng::set_opt_reconnect_time_min(*sockets->video_send, 10);
                nng::set_opt_reconnect_time_max(*sockets->video_send, 10);
                std::string video_send_addr = string_format("tcp://%s:%s", cfgs->video_send_ip.c_str(), cfgs->video_send_port.c_str());
                // PRINT_LOG("video_send_addr: %s\n", video_send_addr.c_str());
                sockets->video_send->dial(video_send_addr.c_str(), nng::flag::nonblock);
                return -1;
            }

            sockets->video_send->send(nng::view(data, size));
            
            // CALC_FPS("sample_sock_video_send");

            PRINT_LOG("sample_sock_video_send %d\n", size);
            
            sockets->video_send_state = 1;
            
        }
        catch (const nng::exception &e)
        {
            PRINT_LOG("sample_sock_video_send %s: %s\n", e.who(), e.what());
            sockets->video_send_state = 0;
            return -1;
        }
        return 0;
    }

    static int sample_sock_video_recv(uint8_t *data, int size)
    {
        try
        {
            if (!cfgs->has_cfg) { usleep(40*1000); return -1; }
        
            if (!sockets->video_recv)
            {
                sockets->video_recv = new nng::socket();
                *sockets->video_recv = nng::pull::open();
                nng::set_opt_recv_timeout(*sockets->video_recv, 40);
                // nng::set_opt_recv_timeout(*sockets->video_recv, 100); // only recv
                nng::set_opt_reconnect_time_min(*sockets->video_recv, 10);
                nng::set_opt_reconnect_time_max(*sockets->video_recv, 10);
                std::string video_recv_addr = string_format("tcp://%s:%s", cfgs->video_recv_ip.c_str(), cfgs->video_recv_port.c_str());
                printf("video_recv_addr: %s\n", video_recv_addr.c_str());
                sockets->video_recv->listen(video_recv_addr.c_str());
                return -1;
            }

            size = sockets->video_recv->recv(nng::view(data, size));

            // CALC_FPS("sample_sock_video_recv");
            
            PRINT_LOG("sample_sock_video_recv %d\n", size);

            sockets->video_recv_state = 1;
        }
        catch (const nng::exception &e)
        {
            PRINT_LOG("sample_sock_video_recv %s: %s\n", e.who(), e.what());
            sockets->video_recv_state = 0;
            return -1;
        }
        return size;
    }

}

int dhv_main(int argc, char **argv)
{
    system("cat /dev/zero > /dev/fb0");
    PRINT_LOG("Hello, dhv_main!\n");

    if (argc == 3)
    {
        sockets->DLS_VO = atoi(argv[1]);
        sockets->DLS_VI = atoi(argv[2]);
    }
    else
    {
        sockets->DLS_VO = 0;
        sockets->DLS_VI = 0;
    }
    PRINT_LOG("DLS_VO = %d, DLS_VI = %d\n", sockets->DLS_VO, sockets->DLS_VI);

    int sample_sock_audio_recv(uint8_t *data, int size);
    int sample_sock_audio_send(uint8_t *data, int size);

    int sample_sock_video_recv(uint8_t *data, int size);
    int sample_sock_video_send(uint8_t *data, int size);

    set_sample_dls_audio_callback(sample_sock_audio_send, sample_sock_audio_recv);
    set_sample_dls_video_callback(sample_sock_video_send, sample_sock_video_recv);

    sample_nng_load(sockets->DLS_VO, sockets->DLS_VI);

    while (!get_sample_nng_exit_flag())
    {
        usleep(250 * 1000);
        sockets->sample_socket_loop();
        cfgs->sample_cfg_loop();
    }

    sample_nng_free(sockets->DLS_VO, sockets->DLS_VI);

    return 0;
}

// 全功能测试
// export IP=0.0.0.0 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./demo 1 1

// 触发抓拍（默认 3 张）
// export IP=0.0.0.0 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","capture"]' > /tmp/sample_cfg.json

// 后板配置 192.168.1.1
// export IP=192.168.1.2 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./demo 1 0

// 前板配置 192.168.1.2
// export IP=192.168.1.1 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./demo 0 1
