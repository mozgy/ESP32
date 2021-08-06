#ifndef _VARS_H_
#define _VARS_H_

#define USE_BUILTIN_WEBSERVER
#undef USE_ASYNC_WEBSERVER

#define USE_BUILTIN_OTA
#undef USE_ELEGANT_OTA

#if defined (USE_BUILTIN_WEBSERVER) && defined (USE_ASYNC_WEBSERVER)
#error "Cannot use both defines USE_BUILTIN_WEBSERVER and USE_ASYNC_WEBSERVER"
#endif

#if defined (USE_BUILTIN_WEBSERVER) && defined (USE_ELEGANT_OTA)
#error "Elegant OTA library requires AsyncWebServer"
#endif


/*
typedef enum {
    FRAMESIZE_96X96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240X240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    // 3MP Sensors
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_P_HD,     //  720x1280
    FRAMESIZE_P_3MP,    //  864x1536
    FRAMESIZE_QXGA,     // 2048x1536
    // 5MP Sensors
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_P_FHD,    // 1080x1920
    FRAMESIZE_QSXGA,    // 2560x1920
    FRAMESIZE_INVALID
} framesize_t;
 */

framesize_t picSnapSize = FRAMESIZE_QVGA;
typedef const String picSizeStrings_t;

// foo has to be of framesize_t size
picSizeStrings_t foo[] = {
  "Framesize 96x96",
  "Framesize QQVGA - 160x120",
  "Framesize QCIF - 176x144",
  "Framesize HQVGA - 240x176",
  "Framesize 240x240",
  "Framesize QVGA - 320x240",
  "Framesize CIF - 400x296",
  "Framesize HVGA - 480x320",
  "Framesize VGA - 640x480",
  "Framesize SVGA - 800x600",
  "Framesize XGA - 1024x768",
  "Framesize HD",
  "Framesize SXGA - 1280x1024",
  "Framesize UXGA - 1600x1200",
  "Framesize FHD",
  "Framesize P_HD",
  "Framesize P_3MP",
  "Framesize QXGA - 2048x1536"
};

bool flashEnable = false;

#define WIFI_DISC_DELAY 30000L
unsigned long wifiWaitTime;
int wifiSTATries;

long timeZone = 1;
byte daySaveTime = 1;
struct tm tmstruct;

char elapsedTimeString[40];
char currentDateTime[17];

#endif
