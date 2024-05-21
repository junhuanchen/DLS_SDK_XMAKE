
#include <linux/input.h>
#include "linux/watchdog.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief 看门狗初始化
 * @details
 * 目前看门狗超时时间只能设置为1、2、3、4、5、6、8、10、12、14、16秒
 * @param [in] feed_time          喂狗时间，单位s
 * @retval
 */
static void _watchdog_init(int feed_time)
{
    int res;
    int fd = -1;

    fd = open("/dev/watchdog", O_RDWR);
    if (fd < 0)
    {
        // fprintf(stderr, "open %s error:%s\n", "/dev/watchdog", strerror(errno));
        return;
    }

    res = ioctl(fd, WDIOC_SETTIMEOUT, &feed_time);
    if (res < 0)
    {
        fprintf(stderr, "watchdog set timeout error\n");
        close(fd);
        return;
    }

    res = ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
    if (res < 0)
    {
        fprintf(stderr, "watchdog enable error\n");
        close(fd);
        return;
    }

    res = close(fd);
    if (res < 0)
    {
        // fprintf(stderr, "close %s error:%s\n", "/dev/watchdog", strerror(errno));
        return;
    }
}

static void _watchdog_feed(void)
{
    int res;
    int fd = -1;

    fd = open("/dev/watchdog", O_RDWR);
    if (fd < 0)
    {
        // fprintf(stderr, "open %s error:%s\n", "/dev/watchdog", strerror(errno));
        return;
    }

    res = ioctl(fd, WDIOC_KEEPALIVE, 0);
    if (res < 0)
    {
        fprintf(stderr, "watchdog feed error\n");
        close(fd);
        return;
    }

    res = close(fd);
    if (res < 0)
    {
        // fprintf(stderr, "close %s error:%s\n", "/dev/watchdog", strerror(errno));
        return;
    }
}
