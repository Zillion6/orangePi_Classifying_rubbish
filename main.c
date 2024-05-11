#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "rubbish.h"
#include "mySerial.h"
#include "pwm.h"
#include "myOLED.h"
#include "socket_Sever.h"

int len = -1;
int serial_fd = -1;
pthread_mutex_t mutex;
pthread_cond_t  cond;

int detect_process(char *process_name)
{
    FILE *file;
    int pid;
    char cmd[64] = {'\0'};
    char buffer[128] = {'\0'};
    
    sprintf(cmd, "ps -ax | grep  %s | grep -v grep", process_name);
    file = popen(cmd, "r");
    if(file != NULL)
    {
        if(fgets(buffer, sizeof(buffer), file) != NULL)
        {
            pid = atoi(buffer);

            return pid;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    pclose(file);
    return pid;
}

void* send_voice(void *arg)
{
    printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    pthread_detach(pthread_self());
    unsigned char *buffer = (unsigned char *)arg;

    printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);

    if(NULL != buffer)
    {
        my_serialPuts(serial_fd, buffer, 6);

        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    }

    pthread_exit(0);
}

void* trash_can(void *arg)
{
    pthread_detach(pthread_self());
    unsigned char *buffer = (unsigned char *)arg;
    
    printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);

    if(buffer[2] == 0x43)
    {
        pwm_write(PWM_RECOVERABLE_RUBBISH);
        delay(2300);
        pwm_stop(PWM_RECOVERABLE_RUBBISH);

        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    }
    else if(buffer[2] != 0x45)
    {
        pwm_write(PWM_OTHER_RUBBISH);
        delay(2300);
        pwm_stop(PWM_OTHER_RUBBISH);

        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    }

    pthread_exit(0);
}

void* myOled_show(void *arg)
{
   pthread_detach(pthread_self());
   myOled_init();
   oled_show(arg);
   pthread_exit(0);
}

void* get_voice()
{   

    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0X55, 0xAA};
    while(1)
    {
        len = my_serialGets(serial_fd, buffer);

        if(len > 0 && buffer[2] == 0x46)
        {
//            system(WGET_CMD);
            pthread_mutex_lock(&mutex);
            buffer[2] = 0x00;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(0);
}

void* get_category()
{
    char *category = NULL;
    pthread_t send_voice_tid;
    pthread_t trash_can_tid;
    pthread_t myOled_show_tid;
    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0X55, 0xAA};
    
    printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    
    while(1)
    {
        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
        
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);

        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
        
        buffer[2] = 0x00;
        system(WGET_CMD);
        
        if(access(RUBBISH_FILE, F_OK) == 0)
        {   
            category = alicloud_rubbish_category(category);
            printf("category = %s\n", category);
        
            if(strstr(category, "干垃圾") != NULL)
            {
                buffer[2] = 0x41;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }
        
            else if(strstr(category, "湿垃圾"))
            {
                buffer[2] = 0x42;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }
        
            else if(strstr(category, "可回收垃圾"))
            {
                buffer[2] = 0x43;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }
        
            else if(strstr(category, "有害垃圾"))
            {
                buffer[2] = 0x44;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }
        
            else
            {
                buffer[2] = 0x45;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }
            //printf("ret_strstr = %s\n", ret_strstr);
        }
        else
        {
            buffer[2] = 0x45;
            printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
            //my_serialPuts(serial_fd, buffer, 6);
        }
        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
    
        if(pthread_create(&send_voice_tid, NULL, send_voice, buffer) != 0)
        {
            perror("pthread_create1");
            exit(-1);
        }
        
        if(pthread_create(&trash_can_tid, NULL, trash_can, buffer) != 0)
        {
            perror("pthread_create2");
            exit(-1);
        }

        if(pthread_create(&myOled_show_tid, NULL, myOled_show, buffer) != 0)
        {
            perror("pthread_create2");
            exit(-1);
        }

        remove(RUBBISH_FILE);
        
        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);
        
    }
    
    pthread_exit(0);
}

void* socket_sever()
{
    int s_fd;
    int c_fd;
    ssize_t n_receive;
    struct sockaddr_in c_addr;
    char receive_buf[8] = {'\0'};

    memset(&c_addr, 0, sizeof(struct sockaddr_in));

    s_fd = socket_init();

    printf("%s|%s|%d:s_fd=%d\n", __FILE__, __func__, __LINE__, s_fd); 

    sleep(3);

    int addrlen = sizeof(struct sockaddr_in);

    while(1)
    {   
        memset(receive_buf, '\0', sizeof(receive_buf));

        c_fd = accept(s_fd, (struct sockaddr *)&c_addr, &addrlen);
        if(c_fd == -1)
        {
            perror("accept");
            
            continue;
        }
        
        int keepalive = 1;          // 开启TCP KeepAlive功能
        int keepidle = 5;           // tcp_keepalive_time 3s内没收到数据开始发送心跳包
        int keepcnt = 3;            // tcp_keepalive_probes 每次发送心跳包的时间间隔,单位秒
        int keepintvl = 3;          // tcp_keepalive_intvl 每3s发送一次心跳包
        setsockopt(c_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
        setsockopt(c_fd, SOL_TCP, TCP_KEEPIDLE, (void *) &keepidle, sizeof(keepidle));
        setsockopt(c_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt));
        setsockopt(c_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl));
        
        printf("%s|%s|%d: Accept a connection from %s : %d\n", __FILE__, __func__, __LINE__, inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        
        //5.read
        while(1)
        {   
            memset(receive_buf, '\0', sizeof(receive_buf));
            n_receive = recv(c_fd, receive_buf, sizeof(receive_buf), 0);
            printf("%s|%s|%d : %s\n", __FILE__, __func__, __LINE__, receive_buf);
            
            if(n_receive > 0)
            {
                if(strstr(receive_buf, "open"))
                {   
                    printf("%s|%s|%d\n", __FILE__, __func__, __LINE__);
                    pthread_mutex_lock(&mutex);
                    pthread_cond_signal(&cond);
                    pthread_mutex_unlock(&mutex);
                    
                    printf("%s|%s|%d\n", __FILE__, __func__, __LINE__);
                }
            }
            /*
            if(strstr(receive_buf, "open") && n_receive > 0)
            {
                pthread_mutex_lock(&mutex);
                pthread_cond_(&cond);
                pthread_mutex_unlock(&mutex);
            }
            */
            
            else if(0 == n_receive || -1 == n_receive)
            {
                break;
            }
        }
        close(c_fd);
    }
    pthread_exit(0);
}

int main()
{
    int result_detect_process = -1;
    char *ret_strstr = (char *)malloc(1024);
    pthread_t get_voice_tid;
    pthread_t get_category_tid;
    pthread_t socket_sever_tid;

    rubbish_Init();
    if (wiringPiSetup() == -1)
    {
        exit(1);
    }

    result_detect_process = detect_process("mjpg_streamer");
    if(-1 == result_detect_process)
    {
        perror("detect_process");
        
        goto END;
    }

    serial_fd = my_serialOpen(SERIAL_DEV, BAUD);
    if(-1 == serial_fd)
    {
        perror("my_serialOpen");

        goto END;
    }

    pthread_create(&get_voice_tid, NULL, get_voice, NULL);
    pthread_create(&get_category_tid, NULL, get_category, NULL);
    pthread_create(&socket_sever_tid, NULL, socket_sever, NULL);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_join(get_voice_tid, NULL);
    pthread_join(get_category_tid, NULL);
    pthread_join(socket_sever_tid, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    close(serial_fd);

END:
    rubbish_Finalize();
    return 0;
}

#if 0

int detect_process(char *process_name)
{
    FILE *file;
    int pid;
    char cmd[64] = {'\0'};
    char buffer[128] = {'\0'};
    
    sprintf(cmd, "ps -ax | grep  %s | grep -v grep", process_name);
    file = popen(cmd, "r");
    if(file != NULL)
    {
        if(fgets(buffer, sizeof(buffer), file) != NULL)
        {
            pid = atoi(buffer);

            return pid;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    pclose(file);
    return pid;
}

int main()
{
    int len = -1;
    int serial_fd = -1;
    int result_detect_process = -1;
    char *category = NULL;
    char *ret_strstr = (char *)malloc(1024);

    rubbish_Init();
    if (wiringPiSetup() == -1)
    {
        exit(1);
    }

    result_detect_process = detect_process("mjpg_streamer");
    if(-1 == result_detect_process)
    {
        perror("detect_process");
        goto END;
    }

    serial_fd = my_serialOpen(SERIAL_DEV, BAUD);
    if(-1 == serial_fd)
    {
        perror("my_serialOpen");

        goto END;
    }

    while(1)
    {
        len = my_serialGets(serial_fd, buffer);

        if(len > 0 && buffer[2] == 0x46)
        {
            buffer[2] = 0x00;
            system(WGET_CMD);

            printf("------------------------------------\n");
            if(access(RUBBISH_FILE, F_OK) == 0)
            {   
                printf("====================================\n");
                category = alicloud_rubbish_category(category);
                printf("category = %s\n", category);

                if(strstr(category, "干垃圾") != NULL)
                {
                    buffer[2] = 0x41;
                    printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                    //my_serialPuts(serial_fd, buffer, 6);
                }

                else if(strstr(category, "湿垃圾"))
                {
                    buffer[2] = 0x42;
                    printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                    //my_serialPuts(serial_fd, buffer, 6);
                }

                else if(strstr(category, "可回收垃圾"))
                {
                    buffer[2] = 0x43;
                    printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                    //my_serialPuts(serial_fd, buffer, 6);
                }

                else if(strstr(category, "有害垃圾"))
                {
                    buffer[2] = 0x44;
                    printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                    //my_serialPuts(serial_fd, buffer, 6);
                }

                else
                {
                    buffer[2] = 0x45;
                    printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                    //my_serialPuts(serial_fd, buffer, 6);
                }
                //printf("ret_strstr = %s\n", ret_strstr);
            }
            else
            {
                buffer[2] = 0x45;
                printf("[%d]: 0x%2X\n", __LINE__, buffer[2]);
                //my_serialPuts(serial_fd, buffer, 6);
            }

            my_serialPuts(serial_fd, buffer, 6);

            if(buffer[2] == 0x43)
            {
                pwm_write(PWM_RECOVERABLE_RUBBISH);
                delay(2300);
                pwm_stop(PWM_RECOVERABLE_RUBBISH);
            }
            else if(buffer[2] != 0x45)
            {
                pwm_write(PWM_OTHER_RUBBISH);
                delay(2300);
                pwm_stop(PWM_OTHER_RUBBISH);
            }

            buffer[2] = 0x00;
            remove(RUBBISH_FILE);
        }
    }

END:
    rubbish_Finalize();
    return 0;
}

#endif