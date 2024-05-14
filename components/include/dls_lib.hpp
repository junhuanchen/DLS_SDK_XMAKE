
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <inttypes.h>

#include <string>
#include <sstream>
#include <fstream>

#include <string>
#include <list>
#include <condition_variable>
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

static int64_t get_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int64_t set_time_ms(int64_t ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    settimeofday(&tv, NULL);
}

#define CALC_FPS(tips) \
    { \
        static int fcnt = 0; \
        fcnt++; \
        static struct timespec ts1, ts2; \
        clock_gettime(CLOCK_MONOTONIC, &ts2); \
        if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
        { \
            printf("%s => FPS:%d \r\n", tips, fcnt); \
            ts1 = ts2; \
            fcnt = 0; \
        } \
    }
