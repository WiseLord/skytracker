#ifndef APP_H
#define APP_H

#include <stdbool.h>

typedef struct {
    bool backlight;
    bool clear;
} App;

void appInit(void);
void appRun(void);

void appActionGet(void);
void appActionRemap(void);
void appActionHandle(void);

void appShow(void);

#endif // APP_H
