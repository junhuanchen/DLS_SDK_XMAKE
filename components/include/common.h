#ifndef __COMMON_H
#define __COMMON_H

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include <string>
#include <list>
#include <vector>
#include <map>
#include <unistd.h>



// #include "config.h"
//#include <openssl/objects.h>
//#include <openssl/rsa.h>
//#include <openssl/pem.h>
//#include <openssl/bio.h>
//#include <openssl/err.h>

#define SAFE_RELEASE(a) \
    do                  \
    {                   \
        if (a)          \
        {               \
            delete (a); \
            (a) = NULL; \
        }               \
    } while (0)
#define SAFE_RELEASE_ARRAY(a) \
    do                        \
    {                         \
        if (a)                \
        {                     \
            delete[] (a);     \
            (a) = NULL;       \
        }                     \
    } while (0)




#endif
// #include "XQTools.h"
// #include "XQLock.h"

