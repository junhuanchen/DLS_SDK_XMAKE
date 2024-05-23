
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

// cd /sys/kernel/debug/sunxi_pinctrl/ && echo PC5 0 > function && echo PC5 > sunxi_pin && cat data
int gpio_read(std::string gpio="PC4")
{
    system(string_format("cd /sys/kernel/debug/sunxi_pinctrl/ && echo %s 0 > function && echo %s > sunxi_pin && cat data > /tmp/data", gpio.c_str(), gpio.c_str()).c_str());
    // 解析 /tmp/data 的 "pin[PC5] data: 1 \n" 获取倒数第三个字节，转换成数值
    std::string data = string_read_file("/tmp/data");
    // printf("[%s]\n", data.c_str());
    return atoi(data.substr(data.length() - 3, 3).c_str());
}

// cd /sys/kernel/debug/sunxi_pinctrl/ && echo PE10 1 >function && echo PE10 0 >data
void gpio_write(std::string gpio="PC4", int val=1, int pull=0)
{
    system(string_format("cd /sys/kernel/debug/sunxi_pinctrl/ && echo %s 1 > function && echo %s %d > data", gpio.c_str(), gpio.c_str(), val).c_str());
}

void gpio_test()
{
    gpio_write("PC4", 0);
    printf("GPIO PC5: %d\n", gpio_read("PC5"));
    usleep(100*1000);
    gpio_write("PC4", 1);
    printf("GPIO PC5: %d\n", gpio_read("PC5"));
    usleep(100*1000);
    gpio_write("PC4", 0);
    printf("GPIO PC5: %d\n", gpio_read("PC5"));
    usleep(100*1000);
    gpio_write("PC4", 1);
    printf("GPIO PC5: %d\n", gpio_read("PC5"));
    usleep(100*1000);
    gpio_write("PH0", 0);
    usleep(100*1000);
    gpio_write("PH0", 1);
    usleep(100*1000);
    gpio_write("PH0", 0);
    usleep(100*1000);
    gpio_write("PH0", 1);
    usleep(100*1000);
}
