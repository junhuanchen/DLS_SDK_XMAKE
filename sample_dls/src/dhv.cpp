
#include "sample_common.h"

#include <unistd.h>
#include <string>
#include <sstream>
#include <list>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include "json5pp.hpp"

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

#include "TCPServer.h"
#include "TCPClient.h"

auto LogPrinter = [](const std::string &strLogMsg)
{ std::cout << strLogMsg << std::endl; };

struct _sample_hv_
{
    bool is_init = false;

    // uint8_t state = 0, work = 0;
    std::string audio_recv_addr = "0.0.0.0:1028";
    std::string audio_send_addr = "0.0.0.0:1028";

    CTCPClient *audio_recv = nullptr;
    CTCPServer *audio_send = nullptr;

    std::string video_recv_addr = "0.0.0.0:2810";
    std::string video_send_addr = "0.0.0.0:2810";

    CTCPClient *video_recv = nullptr;
    CTCPServer *video_send = nullptr;

    void get_addr_port(std::string addr, std::string &ip, std::string &port)
    {
        std::string::size_type pos = addr.find_last_of(':');
        port = addr.substr(0, pos);
        ip = addr.substr(pos + 1);
        printf("ip %s port %s\n", ip.c_str(), port.c_str());
    }

    void sample_hv_init(std::string audio_recv_addr, std::string video_recv_addr, std::string audio_send_addr, std::string video_send_addr)
    {
        sample_hv_exit();
        if (is_init == false)
        {
            if (audio_recv)
            {
                std::string ip;
                std::string port;
                get_addr_port(audio_recv_addr, ip, port);
            }
            is_init = true;
        }
    }

    void sample_hv_exit()
    {
        if (is_init)
        {

            is_init = false;
        }
    }

    void sample_hv_loop()
    {
        // printf("sample_data: %d %d %d %d\r\n", sample_data.audio_recv_state, sample_data.audio_send_state, sample_data.video_recv_state, sample_data.video_send_state);
        // uint8_t *buf = (uint8_t *)&sample_data;
        // uint32_t len = sizeof(sample_data);
        // sample_hv.system_sock.send(nng::view(buf, len));
        // print sample_data

        if (access("/tmp/sample_hv.json", F_OK) == 0)
        {
            // echo '["0.0.0.0:1031","0.0.0.0:2813","0.0.0.0:1030","0.0.0.0:2812","work"]' > /tmp/sample_hv.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","stop"]' > /tmp/sample_hv.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","work"]' > /tmp/sample_hv.json
            // echo '["0.0.0.0:1030","0.0.0.0:2812","0.0.0.0:1030","0.0.0.0:2812","exit"]' > /tmp/sample_hv.json
            auto conf = string_read_file("/tmp/sample_hv.json");

            try
            {
                auto cfg = json5pp::parse(conf);
                auto audio_recv_addr = cfg.as_array().at(0).as_string();
                auto video_recv_addr = cfg.as_array().at(1).as_string();
                auto audio_send_addr = cfg.as_array().at(2).as_string();
                auto video_send_addr = cfg.as_array().at(3).as_string();
                auto user_data = cfg.as_array().at(4).as_string();

                printf("user_data: %s\n", user_data.c_str());
                if (user_data == "stop")
                {
                    sample_hv_exit();
                }
                else if (user_data == "exit")
                {
                    extern void sample_nng_signalHandler(int signum);
                    sample_nng_signalHandler(0);
                }
                else if (user_data == "load")
                {
                    sample_hv_init(audio_recv_addr, video_recv_addr, audio_send_addr, video_send_addr);
                }
            }
            catch (json5pp::syntax_error e)
            {
                printf("load_json_conf %s : %s", conf.c_str(), e.what());
            }

            system("rm /tmp/sample_hv.json");
        }

        system("echo 3 > /proc/sys/vm/drop_caches");

        if (is_init == false) // 如果上电没有配置项，为了初始化，自己生成。
        {
            is_init = true;
            system("echo '[\"0.0.0.0:1030\",\"0.0.0.0:2812\",\"0.0.0.0:1030\",\"0.0.0.0:2812\",\"load\"]' > /tmp/sample_hv.json");
        }
    }

} sample_hv;

extern "C"
{

    static int sample_hv_audio_recv(uint8_t *data, int size)
    {
        // try
        // {
        //   if (sample_hv.audio_recv)
        //   {
        //     sample_hv.audio_recv->send(nng::view(data, size));
        //     // CALC_FPS("sample_hv_audio_recv");
        //   }
        //   else
        //   {
        //     usleep(20*1000);
        //   }
        // }
        // catch (const nng::exception &e)
        // {
        //   // printf("audio_recv %s: %s\n", e.who(), e.what());
        //   return -1;
        // }
        return 0;
    }

    static int sample_hv_audio_send(uint8_t *data, int size)
    {
        // try
        // {
        //   if (sample_hv.audio_send)
        //   {
        //     size = sample_hv.audio_send->recv(nng::view(data, size));
        //     // CALC_FPS("sample_hv_audio_send");
        //   }
        //   else
        //   {
        //     usleep(20*1000);
        //   }
        // }
        // catch (const nng::exception &e)
        // {
        //   // printf("audio_send %s: %s\n", e.who(), e.what());
        //   return -1;
        // }
        return size;
    }

    static int sample_hv_video_recv(uint8_t *data, int size)
    {
        // try
        // {
        //   if (sample_hv.video_recv)
        //   {
        //     sample_hv.video_recv->send(nng::view(data, size));
        //     // CALC_FPS("sample_hv_video_recv");
        //   }
        //   else
        //   {
        //     usleep(20*1000);
        //   }
        //   // printf("sample_hv_video_recv %d\n", size);
        // }
        // catch (const nng::exception &e)
        // {
        //   // printf("video_recv %s: %s\n", e.who(), e.what());
        //   return -1;
        // }
        return 0;
    }

    static int sample_hv_video_send(uint8_t *data, int size)
    {
        // try
        // {
        //   if (sample_hv.video_send)
        //   {
        //     size = sample_hv.video_send->recv(nng::view(data, size));
        //     // CALC_FPS("sample_hv_video_send");
        //   }
        //   else
        //   {
        //     usleep(20*1000);
        //   }
        //   // printf("sample_hv_video_send %d\n", size);
        // }
        // catch (const nng::exception &e)
        // {
        //   // printf("video_send %s: %s\n", e.who(), e.what());
        //   return -1;
        // }
        return size;
    }
}

int dhv_main(int argc, char **argv)
{
    system("cat /dev/zero > /dev/fb0");
    printf("Hello, dhv_main!\n");

    int DLS_VO = 0;
    int DLS_VI = 0;
    if (argc == 3)
    {
        DLS_VO = atoi(argv[1]);
        DLS_VI = atoi(argv[2]);
    }
    else
    {
        DLS_VO = 0;
        DLS_VI = 0;
    }
    printf("DLS_VO = %d, DLS_VI = %d\n", DLS_VO, DLS_VI);

    // int sample_hv_audio_recv(uint8_t *data, int size);
    // int sample_hv_audio_send(uint8_t *data, int size);
    // set_sample_dls_audio_callback(sample_hv_audio_recv, sample_hv_audio_send);

    // int sample_hv_video_recv(uint8_t *data, int size);
    // int sample_hv_video_send(uint8_t *data, int size);
    // set_sample_dls_video_callback(sample_hv_video_recv, sample_hv_video_send);

    // sample_nng_load(DLS_VO, DLS_VI);

    // while (!get_sample_nng_exit_flag())
    for (int i = 0; i < 10; i++)
    {
        usleep(1000 * 1000);
        sample_hv.sample_hv_loop();
    }

    // sample_nng_free(DLS_VO, DLS_VI);

    return 0;
}
