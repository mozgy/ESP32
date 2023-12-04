#ifndef _MYVARS_H_
#define _MYVARS_H_

#include <Arduino.h>

#define SW_VERSION "0.15.3"

#define HAVE_CAMERA
#define ESP_CAM_HOSTNAME "mozz-cam"
#define AI_CAM_SERIAL "1"

#define FLASHENABLED true
#define TIMELAPSE false

#define DBG_OUTPUT_PORT Serial

extern String photoFrame;
extern bool flashEnabled;
extern bool timeLapse;

extern long timeZone;
extern byte daySaveTime;

extern int waitTime;

extern char elapsedTimeString[40];
extern char currentDateTime[17];

void fnSetFrameSize( String frameSize );

#endif
