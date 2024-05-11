#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "oled.h"
#include "font.h"
#include "myOLED.h"

struct display_info disp;

void oled_show(void *arg)
{
    int i;
    unsigned char *buffer = (unsigned char *)arg;

    oled_putstrto(&disp, 1, 9+1, "Rubbish recognition : ");
    disp.font = font2;
    oled_send_buffer(&disp);

    switch(buffer[2])
    {
        case 0x41:
                    oled_putstrto(&disp, 3, 18+2, "dry rubbish");
            break;
        
        case 0x42:
                    oled_putstrto(&disp, 1, 18+2, "wet rubbish");
            break;
        
        case 0x43:
                    oled_putstrto(&disp, 1, 18+2, "recyclable rubbish");
            break;

        case 0x44:
                    oled_putstrto(&disp, 1, 18+2, "hazardous rubbish");
            break;
        
        case 0x45:
                    oled_putstrto(&disp, 1, 18+2, "recognition failure");
            break;

        default:
            break;
    }
    disp.font = font2;
    oled_send_buffer(&disp);

    disp.font = font3;
    for (i = 0; i < 100; i++) {
		oled_putstrto(&disp, 135-i, 36+4, "===");
		oled_send_buffer(&disp);
	}

}

void show_error(int err, int add)
{
	printf("\nERROR: %i, %i\n\n", err, add);
}

void myOled_init(void)
{
    int e;
    disp.address = OLED_I2C_ADDR;
    disp.font = font2;

    e = oled_open(&disp, FILENAME);
    if (e < 0) 
    {
		show_error(1, e);
	}
    else
    {
		e = oled_init(&disp);
    }
}