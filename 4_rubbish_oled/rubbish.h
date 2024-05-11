#ifndef _RUBBISH_H

#define _RUBBISH_H

#define WGET_CMD "wget http://127.0.0.1:8080/?action=snapshot -O /home/orangepi/Project/rubbish/rubbish.jpg"

#define RUBBISH_FILE "/home/orangepi/Project/rubbish/rubbish.jpg"

void rubbish_Init(void);

void rubbish_Finalize(void);

char *alicloud_rubbish_category(char *category);

#endif