#include <iostream>

#include "sqlite3.h"

using namespace std;

extern "C"
{
    int test_log()
    {
        cout << "test_log" << endl;
        printf("src Hello World!\n");
        puts(SQLITE_VERSION);
        return 0;
    }
}
