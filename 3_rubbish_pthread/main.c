#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include "rubbish.h"
#include "mySerial.h"
#include "pwm.h"

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
    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0X55, 0xAA};
    
    while(1)
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        
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
        remove(RUBBISH_FILE);
        
        printf("%s|%s|%d: \n", __FILE__, __func__, __LINE__);

    }
    
    pthread_exit(0);
}

int main()
{
    int result_detect_process = -1;
    char *ret_strstr = (char *)malloc(1024);
    pthread_t get_voice_tid;
    pthread_t get_category_tid;

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

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_join(get_voice_tid, NULL);
    pthread_join(get_category_tid, NULL);

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