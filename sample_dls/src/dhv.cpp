
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

#include "hv/TcpServer.h"
#include "hv/TcpClient.h"
#include "hv/htime.h"
#include <string>
#include <queue>

using namespace hv;


#define TEST_RECONNECT  1

int TcpClientMain(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s remote_port [remote_host]\n", argv[0]);
        return -10;
    }
    int remote_port = atoi(argv[1]);
    const char* remote_host = "127.0.0.1";
    if (argc > 2) {
        remote_host = argv[2];
    }

    TcpClient cli;
    int connfd = cli.createsocket(remote_port, remote_host);
    if (connfd < 0) {
        return -20;
    }
    printf("client connect to port %d, connfd=%d ...\n", remote_port, connfd);
    cli.onConnection = [&cli](const SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            printf("connected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
            // send(time) every 3s
            setInterval(3000, [channel](TimerID timerID){
                if (channel->isConnected()) {
                    if (channel->isWriteComplete()) {
                        char str[DATETIME_FMT_BUFLEN] = {0};
                        datetime_t dt = datetime_now();
                        datetime_fmt(&dt, str);
                        channel->write(str);
                    }
                } else {
                    killTimer(timerID);
                }
            });
        } else {
            printf("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        }
        if (cli.isReconnect()) {
            printf("reconnect cnt=%d, delay=%d\n", cli.reconn_setting->cur_retry_cnt, cli.reconn_setting->cur_delay);
        }
    };
    cli.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
        printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
    };

#if TEST_RECONNECT
    // reconnect: 1,2,4,8,10,10,10...
    reconn_setting_t reconn;
    reconn_setting_init(&reconn);
    reconn.min_delay = 1000;
    reconn.max_delay = 10000;
    reconn.delay_policy = 2;
    cli.setReconnect(&reconn);
#endif

#if TEST_TLS
    cli.withTLS();
#endif

    cli.start();

    std::string str;
    while (std::getline(std::cin, str)) {
        if (str == "close") {
            cli.closesocket();
        } else if (str == "start") {
            cli.start();
        } else if (str == "stop") {
            cli.stop();
            break;
        } else {
            if (!cli.isConnected()) break;
            cli.send(str);
        }
    }

    return 0;
}

int TcpServerMain(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return -10;
    }
    int port = atoi(argv[1]);

    hlog_set_level(LOG_LEVEL_DEBUG);

    TcpServer srv;
    int listenfd = srv.createsocket(port);
    if (listenfd < 0) {
        return -20;
    }
    printf("server listen on port %d, listenfd=%d ...\n", port, listenfd);
    srv.onConnection = [](const SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            printf("%s connected! connfd=%d id=%d tid=%ld\n", peeraddr.c_str(), channel->fd(), channel->id(), currentThreadEventLoop->tid());
        } else {
            printf("%s disconnected! connfd=%d id=%d tid=%ld\n", peeraddr.c_str(), channel->fd(), channel->id(), currentThreadEventLoop->tid());
        }
    };
    srv.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
        // echo
        printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
        channel->write(buf);
    };
    srv.setThreadNum(4);
    srv.setLoadBalance(LB_LeastConnections);
    
    srv.start();

    std::string str;
    while (std::getline(std::cin, str)) {
        if (str == "close") {
            srv.closesocket();
        } else if (str == "start") {
            srv.start();
        } else if (str == "stop") {
            srv.stop();
            break;
        } else {
            srv.broadcast(str.data(), str.size());
        }
    }

    return 0;
}

struct _sample_hv_
{
    bool is_init = false;
    
    // https://gitee.com/libhv/libhv/blob/master/evpp/TcpClient_test.cpp
    // https://gitee.com/libhv/libhv/blob/master/evpp/TcpServer_test.cpp

    // uint8_t state = 0, work = 0;
    std::string audio_push_addr = "tcp://0.0.0.0:1028";
    std::string audio_pull_addr = "tcp://0.0.0.0:1028";
    hv::TcpClient audio_push;
    hv::TcpServer audio_pull;
    
    std::string video_push_addr = "tcp://0.0.0.0:2810";
    std::string video_pull_addr = "tcp://0.0.0.0:2810";
    hv::TcpClient video_push;
    hv::TcpServer video_pull;

    std::string get_addr_port(std::string addr)
    {
      std::string::size_type pos = addr.find_last_of(':');
      return addr.substr(0, pos);
    }

    extern "C" void sample_hv_init(std::string audio_push_addr, std::string video_push_addr, std::string audio_pull_addr, std::stringvideo_pull_addr)
    {
      sample_hv_exit();
      if (is_init == false)
      {
        

        is_init = true;
      }
    }

    extern "C" void sample_hv_exit()
    {
      if (is_init)
      {


        is_init = false;
      }
    }

    extern "C" void sample_hv_loop()
    {
      // printf("sample_data: %d %d %d %d\r\n", sample_data.audio_push_state, sample_data.audio_pull_state, sample_data.video_push_state, sample_data.video_pull_state);
      // uint8_t *buf = (uint8_t *)&sample_data;
      // uint32_t len = sizeof(sample_data);
      // sample_hv.system_sock.send(nng::view(buf, len));
      // print sample_data
      
      if (access("/tmp/sample_hv.json", F_OK) == 0)
      {
        // echo '["tcp://0.0.0.0:1031","tcp://0.0.0.0:2813","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","work"]' > /tmp/sample_hv.json
        // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","stop"]' > /tmp/sample_hv.json
        // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","work"]' > /tmp/sample_hv.json
        // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","exit"]' > /tmp/sample_hv.json
        auto conf = string_read_file("/tmp/sample_hv.json");

        try
        {
          auto cfg = json5pp::parse(conf);
          auto audio_push_addr = cfg.as_array().at(0).as_string();
          auto video_push_addr = cfg.as_array().at(1).as_string();
          auto audio_pull_addr = cfg.as_array().at(2).as_string();
          auto video_pull_addr = cfg.as_array().at(3).as_string();
          auto user_data = cfg.as_array().at(4).as_string();

          printf("user_data: %s\n", user_data.c_str());
          if (user_data == "stop")
          {
            sample_hv_free();
          }
          else if (user_data == "exit")
          {
            extern void sample_nng_signalHandler(int signum);
            sample_nng_signalHandler(0);
          }
          else if (user_data == "load")
          {
            sample_hv_load(audio_push_addr, video_push_addr, audio_pull_addr, video_pull_addr); 
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
        system("echo '[\"tcp://0.0.0.0:1030\",\"tcp://0.0.0.0:2812\",\"tcp://0.0.0.0:1030\",\"tcp://0.0.0.0:2812\",\"exit\"]' > /tmp/sample_hv.json");
      }
    }

} sample_hv;

static extern "C" int sample_hv_audio_push(uint8_t *data, int size)
{
  // try
  // {
  //   if (sample_hv.audio_push)
  //   {
  //     sample_hv.audio_push->send(nng::view(data, size));
  //     // CALC_FPS("sample_hv_audio_push");
  //   }
  //   else
  //   {
  //     usleep(20*1000);
  //   }
  // }
  // catch (const nng::exception &e)
  // {
  //   // printf("audio_push %s: %s\n", e.who(), e.what());
  //   return -1;
  // }
  return 0;
}

static extern "C" int sample_hv_audio_pull(uint8_t *data, int size)
{
  // try
  // {
  //   if (sample_hv.audio_pull)
  //   {
  //     size = sample_hv.audio_pull->recv(nng::view(data, size));
  //     // CALC_FPS("sample_hv_audio_pull");
  //   }
  //   else
  //   {
  //     usleep(20*1000);
  //   }
  // }
  // catch (const nng::exception &e)
  // {
  //   // printf("audio_pull %s: %s\n", e.who(), e.what());
  //   return -1;
  // }
  return size;
}

static extern "C" int sample_hv_video_push(uint8_t *data, int size)
{
  // try
  // {
  //   if (sample_hv.video_push)
  //   {
  //     sample_hv.video_push->send(nng::view(data, size));
  //     // CALC_FPS("sample_hv_video_push");
  //   }
  //   else
  //   {
  //     usleep(20*1000);
  //   }
  //   // printf("sample_hv_video_push %d\n", size);
  // }
  // catch (const nng::exception &e)
  // {
  //   // printf("video_push %s: %s\n", e.who(), e.what());
  //   return -1;
  // }
  return 0;
}

static extern "C" int sample_hv_video_pull(uint8_t *data, int size)
{
  // try
  // {
  //   if (sample_hv.video_pull)
  //   {
  //     size = sample_hv.video_pull->recv(nng::view(data, size));
  //     // CALC_FPS("sample_hv_video_pull");
  //   }
  //   else
  //   {
  //     usleep(20*1000);
  //   }
  //   // printf("sample_hv_video_pull %d\n", size);
  // }
  // catch (const nng::exception &e)
  // {
  //   // printf("video_pull %s: %s\n", e.who(), e.what());
  //   return -1;
  // }
  return size;
}

// echo "tcp://0.0.0.0:1028,tcp://0.0.0.0:2810,tcp://0.0.0.0:1028,tcp://0.0.0.0:2810,stop" > /tmp/sample_hv.cfg
// echo "tcp://0.0.0.0:1030,tcp://0.0.0.0:2812,tcp://0.0.0.0:1031,tcp://0.0.0.0:2813,work" > /tmp/sample_hv.cfg
int dhv_main(int argc, char **argv)
{
    system("cat /dev/zero > /dev/fb0");
    printf("Hello, main!\n");

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

    int sample_hv_audio_push(uint8_t *data, int size);
    int sample_hv_audio_pull(uint8_t *data, int size);
    set_sample_dls_audio_callback(sample_hv_audio_push, sample_hv_audio_pull);
    
    int sample_hv_video_push(uint8_t *data, int size);
    int sample_hv_video_pull(uint8_t *data, int size);
    set_sample_dls_video_callback(sample_hv_video_push, sample_hv_video_pull);

    sample_nng_load(DLS_VO, DLS_VI);

    while (!get_sample_nng_exit_flag())
    {
        usleep(1000 * 1000);
        sample_hv.sample_hv_loop();
    }

    sample_nng_free(DLS_VO, DLS_VI);

    return 0;
}

