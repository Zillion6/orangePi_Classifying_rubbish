#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket_Sever.h"

int socket_init()
{
    int s_fd;
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(struct sockaddr_in));

    s_fd = socket(AF_INET,SOCK_STREAM,0);
    if(s_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(atoi(MY_PORT));
    inet_aton(MY_IP_ADDRESS, &s_addr.sin_addr);
    //2.bind
    int result_bind = bind(s_fd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_in));
    if (-1 == result_bind)
    {
        perror("bind");
        return -1;
    }
    //3.listen
    int result_listen = listen(s_fd, 1);
    if (-1 == result_listen)
    {
        perror("listen");
        return -1;
    }

    return s_fd;
}