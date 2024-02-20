#include <iostream>
using namespace std;

extern "C"
{
    int test_log()
    {
        cout << "test_log" << endl;
        return 0;
    }
}
