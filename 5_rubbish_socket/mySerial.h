#ifndef _MYSERIAL_H_
#define _MYSERIAL_H_

#define SERIAL_DEV "/dev/ttyS5"
#define BAUD 115200

int my_serialOpen (const char *device, const int baud);

void my_serialPuts(const int fd, unsigned char *s, int len);

int my_serialGets(const int fd, const char *buffer);

char* serialGetchar(const int fd);

#endif