#ifndef _MYVARS_H_
#define _MYVARS_H_

#include <Arduino.h>

#define SW_VERSION "0.19.17"

#define HAVE_CAMERA
#define ESP_CAM_HOSTNAME "mozz-cam"
#define CAM_SERIAL "X7"

#define FLASH_ENABLED true

#define HAVE_SDCARD
#define TIME_LAPSE_MODE false
#define HIDE_ROOT_DIR false

#define DBG_OUTPUT_PORT Serial

extern String photoFrame;
extern bool timeLapse;

extern long timeZone;
extern byte daySaveTime;

extern int waitTime;

extern char elapsedTimeString[40];
extern char currentDateTime[17];

void fnSetFrameSize( String frameSize );

#endif
