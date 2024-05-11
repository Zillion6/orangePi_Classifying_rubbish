#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>
#include "rubbish.h"
#include "mySerial.h"

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
    unsigned char buffer[6] = {0xAA, 0x55, 0x00, 0x00, 0x55, 0xAA};

    rubbish_Init();
    
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
            buffer[2] = 0x00;
            remove(RUBBISH_FILE);
        }
    }

END:
    rubbish_Finalize();
    return 0;
}