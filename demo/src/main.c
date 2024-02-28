#include "stdio.h"
#include "log.hpp"
#include "sqlite3.h"

// #include "yefiot.h"
// #include "XQUart.h"
int mode = 0;
// yf_param *g_yf_param = NULL;
int read_key_button()
{
    return 0;
}
int main(void)
{
    test_log();
    printf("DLS Hello World!\n");
    puts(SQLITE_VERSION);
    linux_uart_main();
    mode = read_key_button();
    if(mode > 0)
    {
        handle_key_button(mode);
    }
    return 0;
}
