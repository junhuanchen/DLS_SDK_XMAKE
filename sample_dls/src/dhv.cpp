
#include "sample_common.h"

#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <sstream>
#include <list>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <numeric>
#include <array>
#include <memory>
#include <chrono>

#include "json5pp.hpp"

#include "hv/TcpServer.h"

using namespace hv;

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

extern "C" 
{

    int AW_MPI_AI_SuspendAns(int AudioDevId);
    int AW_MPI_AI_ResumeAns(int AudioDevId);
    int AW_MPI_AI_SuspendAec(int AudioDevId);
    int AW_MPI_AI_ResumeAec(int AudioDevId);
    int AW_MPI_AI_SetAgcDb(int AudioDevId, float fDbGain);
    int AW_MPI_AI_GetAgcDb(int AudioDevId, float *pfDbGain);

    void* AgcDbGainAdjustThread(void* argv)
    {
        int *exitFlag = (int*)argv;
        char buf[32] = {0};
        char *ptr;

        while (!*exitFlag) {
            fgets(buf, 32, stdin);
            // setdbgain -5 (-30~0)
            ptr = buf;
            while(*ptr++ != ' ' && *ptr != '\n');
            if (!strncmp(buf, "setdbgain", 10)) {
                AW_MPI_AI_SetAgcDb(0, atof(ptr));
            } else if (!strncmp(buf, "getdbgain", 9)) {
                float dbgain;
                AW_MPI_AI_GetAgcDb(0, &dbgain);
                printf("getAgcDbGain: %f", dbgain);
            } else if (!strncmp(buf, "aec_on", 6)) {
                AW_MPI_AI_ResumeAec(0);
            } else if (!strncmp(buf, "aec_off", 7)) {
                AW_MPI_AI_SuspendAec(0);
            } else if (!strncmp(buf, "ans_on", 6)) {
                AW_MPI_AI_ResumeAns(0);
            } else if (!strncmp(buf, "ans_off", 7)) {
                AW_MPI_AI_SuspendAns(0);
            } else {
                printf("unknow cmd: %s!", buf);
                // printf("help: setdbgain x for set agc db gain");
                // printf("      getdbgain for get agc db gain value");
            }
            memset(buf, 0, 32);
        }
        return NULL;
    }

}

extern "C" void audio_on_clear();

struct _sample_cfg_
{
    bool is_load = false;
    bool has_cfg = false;
    int capture = 0;
    int monitor = 0; // 默认监控模式
    int reload = 0;

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
                else if (user_data == "default")
                {
                    monitor = 0; // 默认模式
                    capture = 0; // 抓拍次数
                    printf("monitor = %d\n", monitor);
                    audio_on_clear();
                }
                else if (user_data == "monitor")
                {
                    monitor = 1; // 监控模式（后板不再同步音频给前板）
                    printf("monitor = %d\n", monitor);
                    audio_on_clear();
                }
                else if (user_data == "reload")
                {
                    sample_cfg_load(audio_send_addr, video_send_addr, audio_recv_addr, video_recv_addr);
                    reload = 1; // 重载IP配置（只允许重载音频发送端）
                    printf("reload = %d\n", reload);
                    audio_on_clear();
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
            system("echo '[\"0.0.0.0:1030\",\"0.0.0.0:2812\",\"0.0.0.0:1030\",\"0.0.0.0:2812\",\"load\"]' > /tmp/sample_cfg.json");
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

    std::queue<std::array<uint8_t, 1024>> audio_recv_data;
    hloop_t* audio_recv_loop = nullptr;
    std::mutex audio_recv_mutex;
    static void audio_on_close(hio_t* io) {
        printf("audio_on_close\n");
        audio_on_clear();
    }

    void audio_on_clear()
    {
        std::lock_guard<std::mutex> locker(audio_recv_mutex);
        // audio_recv_data.clear();
        while (!audio_recv_data.empty()) audio_recv_data.pop();
    }

    static void audio_on_recv(hio_t* io, void* buf, int readbytes) {
        // hio_write(io, buf, readbytes);
        // printf("audio_on_recv %d\n", readbytes);
        if (readbytes < 1024)
        {
            std::lock_guard<std::mutex> locker(audio_recv_mutex);
            std::array<uint8_t, 1024> tmp;
            memcpy(tmp.data(), &readbytes, sizeof(readbytes));
            memcpy(tmp.data() + sizeof(readbytes), buf, readbytes);
            if (audio_recv_data.size() > 32) {
                audio_recv_data.pop();
            }
            audio_recv_data.push(std::move(tmp));
        }
    }

    static void audio_on_accept(hio_t* io) {
        // 设置close回调
        hio_setcb_close(io, audio_on_close);
        // 设置read回调
        hio_setcb_read(io, audio_on_recv);
        // 开始读
        hio_read(io);
    }

    static void audio_on_thread(std::thread **self) {
        // hloop_t* loop = hloop_new(0);
        // hlog_set_handler(network_logger);
        // nlog_listen(loop, DEFAULT_LOG_PORT);
        // htimer_add(loop, timer_write_log, 1000, INFINITE);
        // hloop_run(loop);
        // hloop_free(&loop);
        while (*self)
        {
            audio_recv_loop = hloop_new(0);
            hio_t* audio_recv_listenio = hloop_create_tcp_server(audio_recv_loop, cfgs->audio_recv_ip.c_str(), atoi(cfgs->audio_recv_port.c_str()), audio_on_accept);
            if (audio_recv_listenio != NULL) {
                hloop_run(audio_recv_loop);
            }
            hloop_free(&audio_recv_loop);
            audio_recv_loop = nullptr;
            sleep(1);
            printf("audio_on_thread\n");
        }
    }

    struct _sample_socket_
    {
        std::thread *audio_recv = nullptr;
        // TcpServer *audio_recv = nullptr;
        // SocketChannelPtr audio_recv_channel;
        CTCPClient *audio_send = nullptr;
        // CTCPServer *audio_recv = nullptr;
        // ASocket::Socket audio_ConnectedClient = INVALID_SOCKET;

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
                audio_send = nullptr;
                usleep(40*1000);
                delete bak;
            }

            // if (audio_ConnectedClient != INVALID_SOCKET)
            // {
            //     audio_ConnectedClient = INVALID_SOCKET;
            // }

            if (audio_recv)
            {
                auto bak = audio_recv;
                audio_recv = nullptr;
                usleep(40*1000);
                if (audio_recv_loop) hloop_stop(audio_recv_loop);
                bak->join();
                delete bak;
            }

            if (video_send)
            {
                auto bak = video_send;
                video_send = nullptr;
                usleep(40*1000);
                delete bak;
            }
            
            if (video_recv)
            {
                auto bak = video_recv;
                video_recv = nullptr;
                usleep(40*1000);
                delete bak;
            }
        }
    } sample_socket, *sockets = &sample_socket;

    static int sample_sock_audio_send(uint8_t *data, int size)
    {
        sockets->audio_send_state = 0;

        if (!cfgs->has_cfg) { usleep(40*1000); return -1; }
        if (cfgs->monitor) { usleep(40*1000); return -1; }
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
        else
        {
            if (cfgs->reload)
            {
                cfgs->reload = 0;
                sockets->audio_send->Disconnect();
                printf("reconnect sample_sock_audio_send\n");
            }
            sockets->audio_send_state = 1;
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
        
        return size;
    }

    static int sample_sock_audio_recv(uint8_t *data, int size)
    {
        if (!cfgs->has_cfg) { usleep(40*1000); return -1; }

        PRINT_LOG("sample_sock_audio_recv size = %d\n", size);
        
        sockets->audio_recv_state = 0;
        
        if (sockets->audio_recv == nullptr)
        {
            sockets->audio_recv = new std::thread(audio_on_thread, &sockets->audio_recv);
        }

        // std::lock_guard<std::mutex> locker(audio_recv_mutex);
        // audio_recv_data.push({audio_buf, readbytes});

        if (audio_recv_data.size() > 0)
        {
            auto &tmp = audio_recv_data.front();
            memcpy(&size, tmp.data(), sizeof(size));
            memcpy(data, tmp.data() + sizeof(size), size);
            std::lock_guard<std::mutex> locker(audio_recv_mutex);
            audio_recv_data.pop();
            sockets->audio_recv_state = 1;
            // PRINT_LOG("audio_recv_size = %d\n", size);
        }
        else
        {
            size = -1;
            usleep(40*1000);
        }

        return size;
    }

    /* 执行 shell 命令并返回 stdout */
    static std::string exec_shell(const char* cmd)
    {
        std::array<char, 256> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
        return result;
    }

    /* 1 秒才检查一次 RSSI，其余时间直接给缓存值 */
    static int get_avg_rssi_cached(int avg_window = 5)
    {
        using clock = std::chrono::steady_clock;
        static int cached_rssi = -1;                           // 缓存值
        static clock::time_point last_real_check{};            // 上次真实检查时刻
        const  auto interval = std::chrono::seconds(1);

        auto now = clock::now();
        if (now - last_real_check >= interval)                 // 到点干活
        {
            last_real_check = now;

            /* 下面和原来一样 */
            static std::vector<int> history;
            std::string out = exec_shell("hgpriv hg0 get sta_list");
            std::smatch m;
            int rssi = -1;
            if (std::regex_search(out, m, std::regex(R"(rssi:(-?\d+))")))
                rssi = std::stoi(m[1]);
            if (rssi != -1)
            {
                history.push_back(rssi);
                if ((int)history.size() > avg_window)
                    history.erase(history.begin());
            }
            if (!history.empty())
                cached_rssi = std::accumulate(history.begin(), history.end(), 0)
                            / (int)history.size();
            else
                cached_rssi = -1;

            /* 一秒只打印一次 */
            printf("rssi_avg=%d (window=%d)\n", cached_rssi, (int)history.size());
        }
        return cached_rssi;
    }

    static int sample_sock_video_send(uint8_t *data, int size)
    {    
        static int skip_cnt = 0;                 // 持久化计数器
        int avg_rssi = get_avg_rssi_cached(3);   // 1 秒真查一次
        if (avg_rssi == -1) {                    // 拿不到 STA
            usleep(40*1000);
            return -1;
        }

        /* 1. 停发区 */
        if (avg_rssi < -80) {                    // -90 以外直接停
            usleep(40*1000);
            return -1;
        }

        /* 2. 动态丢帧区：RSSI 越差，周期越大 */
        int period = 1;                          // 默认全发
        if (avg_rssi < -70)       period = 8;    // -80...-89 ：每 8 帧发 1 帧
        else if (avg_rssi < -60)  period = 6;    // -70...-79 ：每 6 帧发 1 帧
        else if (avg_rssi < -50)  period = 4;    // -60...-69 ：每 4 帧发 1 帧
        else if (avg_rssi < -40)  period = 2;    // -50...-59 ：每 2 帧发 1 帧
        // -40...-49 及更好：period=1，全发

        if (++skip_cnt % period != 0) {          // 没到发送周期
            usleep(40*1000);
            return -1;
        }
        
        /* 以下原逻辑未动 */
        try
        {
            if (!cfgs->has_cfg) { usleep(40*1000); return -1; }

            if (cfgs->capture > 0)
            {
                PRINT_LOG("sample_sock_video_send capture\n");
                string_write_file(string_format("./captur_%d.jpg", cfgs->capture),
                                std::string((char *)data, size));
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
            
            CALC_FPS("sample_sock_video_send");

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
    // system("cat /dev/zero > /dev/fb0");
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

    printf("sample_nng_load\n");
    sample_nng_load(sockets->DLS_VO, sockets->DLS_VI);

    usleep(1000 * 1000);

    // pthread_t tid;
    // int exit = 0;
    // pthread_create(&tid, NULL, AgcDbGainAdjustThread, &exit);

    // AW_MPI_AI_SuspendAec(0);
    // AW_MPI_AI_SetAgcDb(0, 30);
 
    while (!get_sample_nng_exit_flag())
    {
        usleep(250 * 1000);
        sockets->sample_socket_loop();
        cfgs->sample_cfg_loop();
    }
    sample_nng_free(sockets->DLS_VO, sockets->DLS_VI);
    printf("sample_nng_free\n");
    return 0;
}

// 全功能测试
// export IP=0.0.0.0 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./demo 1 1

// 触发抓拍（默认 3 张）
// export IP=0.0.0.0 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","capture"]' > /tmp/sample_cfg.json

// 后板配置 192.168.101.1
// export IP=192.168.101.1 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./sample_dls 1 0

// 前板配置 192.168.101.2
// export IP=192.168.101.2 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","load"]' > /tmp/sample_cfg.json && cd /root/bin && ./sample_dls 0 1

// 后板监控模式（默认值）
// export IP=192.168.101.2 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","monitor"]' > /tmp/sample_cfg.json

// 后板对讲模式
// export IP=192.168.101.2 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","default"]' > /tmp/sample_cfg.json

// 重载音频源，允许改变地址（会被后板监控模式屏蔽）
// export IP=192.168.101.2 && echo '["'$IP':1030","'$IP':2812","0.0.0.0:1030","0.0.0.0:2812","reload"]' > /tmp/sample_cfg.json

