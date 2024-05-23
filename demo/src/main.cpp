// #include "dls_gpio.h"
// #include "watchdog.h"

int main(int argc, char **argv)
{
    // gpio_test();
    // watchdog_test();
    extern int dhv_main(int argc, char **argv);
    return dhv_main(argc, argv);
}
