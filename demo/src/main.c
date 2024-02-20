#include "stdio.h"
#include "log.hpp"
#include "sqlite3.h"

int main(void)
{
    test_log();
    printf("DLS Hello World!\n");
    puts(SQLITE_VERSION);
    return 0;
}
