#ifndef _MENU_H_
#define _MENU_H_

#include <Fsm.h>

// Events
#define FIRST_BOOT 0
#define L_PRESSED 1
#define M_PRESSED 2
#define R_PRESSED 3
#define BUTTON_WAKEUP 10
#define TIME_UPDATE 20
#define SERVER_SUCCEED 21
#define SERVER_FAILED 22

extern Fsm menuFsm;
void menuInit();

#endif
