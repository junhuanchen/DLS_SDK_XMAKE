// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/ioctl.h>
// #include <net/if.h>
// #include <netinet/in.h>
// #include "arpa/innet.h"
// // #include <iostream>

// #include "net.h"
// // using namespace std;
// // #include "socket.h"
// // #include "arpa/innet.h"
// int getdevip() {
//     struct ifreq ifr;
//     char *interface = "wlan0"; // 或者您的网络接口名称，如 "wlan0"
//     int fd = socket(AF_INET, SOCK_DGRAM, 0);

//     ifr.ifr_addr.sa_family = AF_INET;
//     strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

//     ioctl(fd, SIOCGIFADDR, &ifr);
//     close(fd);

//     printf("IP address of %s: %s\n", interface, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

//     return 0;
// }