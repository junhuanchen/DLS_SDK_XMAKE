#include "stdio.h"
#include "log.hpp"
#include "sqlite3.h"
#include <stdlib.h>
#include "unit_test_sql.h"
#include "UartFinger.h"
#include "XQDB.h"
#include "cJSON.h"
#include "finger.h"
#include "linux_uart.h"
#include "yefiot.h"
#include "event.h"
// #include "net.h"
yf_param *g_yf = NULL;

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(void)
{
    test_log();
    // getdevip();
    g_yf = new yf_param();
    printf("DLS Hello World!\n");
    puts(SQLITE_VERSION);
    g_yf->getparameter();
    
    while(1)
    {
        printf("-------------\n");
        sleep(5);
    }
    return 0;
}

// static int callback(void *NotUsed, int argc, char **argv, char **azColName){
//    int i;
//    for(i=0; i<argc; i++){
//       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//    }
//    printf("\n");
//    return 0;
// }

// int main(void)
// {
    // test_log();
    // printf("DLS Hello World!\n");
    // puts(SQLITE_VERSION);


    // unit_test_sql();


    // return 0;
// }