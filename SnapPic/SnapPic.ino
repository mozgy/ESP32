//
// (c) mozgy
//
// MIT Licence
//

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD_MMC.h>
// #include "driver/rtc_io.h"

#define SW_VERSION "1.01.34"
#define AI_CAM_SERIAL "1"

#define DBG_OUTPUT_PORT Serial

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
// DATA1 / Flash LED - PIN4
#define FLASH_LED 4
#define FLASH_ENABLE true
bool flashEnable = false;
framesize_t picSnapSize = FRAMESIZE_XGA;
typedef const String picSizeStrings_t;
picSizeStrings_t foo[] = {
  "Framesize QQVGA - 160x120",
  "Framesize QQVGA2 - 128x160",
  "Framesize QCIF - 176x144",
  "Framesize HQVGA - 240x176",
  "Framesize QVGA - 320x240",
  "Framesize CIF - 400x296",
  "Framesize VGA - 640x480",
  "Framesize SVGA - 800x600",
  "Framesize XGA - 1024x768",
  "Framesize SXGA - 1280x1024",
  "Framesize UXGA - 1600x1200",
  "Framesize QXGA - 2048x1536"
};

#include "credentials.h"
#define WIFI_DISC_DELAY 30000L
unsigned long wifiWaitTime;
int wifiSTATries;

long timeZone = 1;
byte daySaveTime = 1;
struct tm tmstruct;

char elapsedTimeString[40];
char currentDateTime[17];

WebServer webServer(80);
AsyncWebServer asyncWebServer(8080);

typedef struct{
  String listJSON;
  int numItems;
} listjson_t;

Ticker tickerSnapPic;
boolean tickerFired;
int waitTime = 60;
int oldTickerValue;

void flagSnapPicTicker( void ) {
  tickerFired = true;
}

void prnEspStats( void ) {

  uint64_t chipid;

  DBG_OUTPUT_PORT.println();
  DBG_OUTPUT_PORT.printf( "Sketch SW version: %s\n", SW_VERSION );
  DBG_OUTPUT_PORT.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  DBG_OUTPUT_PORT.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  DBG_OUTPUT_PORT.printf( "Heap: %u\n", ESP.getFreeHeap() );
  DBG_OUTPUT_PORT.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  DBG_OUTPUT_PORT.printf( "Chip Revision: %u\n", ESP.getChipRevision() );
  DBG_OUTPUT_PORT.printf( "SDK: %s\n", ESP.getSdkVersion() );
//  DBG_OUTPUT_PORT.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  DBG_OUTPUT_PORT.printf( "Flash Size: %u\n", ESP.getFlashChipSize() );
  DBG_OUTPUT_PORT.printf( "Flash Speed: %u\n", ESP.getFlashChipSpeed() );
  DBG_OUTPUT_PORT.printf( "PSRAM Size: %u\n", ESP.getPsramSize() );
//  DBG_OUTPUT_PORT.printf( "Chip ID: %u\n", ESP.getChipId() );
  chipid=ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  DBG_OUTPUT_PORT.printf( "ESP32 Chip ID = %04X", (uint16_t)(chipid>>32) ); //print High 2 bytes
  DBG_OUTPUT_PORT.printf( "%08X\n", (uint32_t)chipid );                     //print Low 4bytes.
//  DBG_OUTPUT_PORT.printf( "Vcc: %u\n", ESP.getVcc() );
  DBG_OUTPUT_PORT.println();

}

void fnElapsedStr( char *str ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( str, "Elapsed " );
  if ( hour == 0 ) {
    sprintf( str, "%s   ", str );
  } else {
    sprintf( str, "%s%2d:", str, hour );
  }
  if ( minute >= 10 ) {
    sprintf( str, "%s%2d:", str, minute );
  } else {
    if ( hour != 0 ) {
      sprintf( str, "%s0%1d:", str, minute );
    } else {
      sprintf( str, "%s ", str );
      if ( minute == 0 ) {
        sprintf( str, "%s  ", str );
      } else {
        sprintf( str, "%s%1d:", str, minute );
      }
    }
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( str, "%s0%1d", str, ( sec % 60 ) );
  } else {
    sprintf( str, "%s%2d", str, ( sec % 60 ) );
  }

}

listjson_t fnJSONList( File listDir ) {

  String strJSON = "[";
  int itemCounter = 0;

  File file = listDir.openNextFile();
  while( file ){
    itemCounter++;
    if( strJSON != "[" ) {
      strJSON += ',';
    }
    strJSON += "{\"name\":\"";
    strJSON += String( file.name() ).substring(1);
    strJSON += "\"}";
    file = listDir.openNextFile();
  }
  strJSON += "]";

  return { strJSON, itemCounter };

}

void fnSetFrameSize( String frameSize ) {

  if( frameSize == "FRAMESIZE_QQVGA" ) {
    picSnapSize = FRAMESIZE_QQVGA;
  } else if( frameSize == "FRAMESIZE_QQVGA2" ) {
    picSnapSize = FRAMESIZE_QQVGA2;
  } else if( frameSize == "FRAMESIZE_QCIF" ) {
    picSnapSize = FRAMESIZE_QCIF;
  } else if( frameSize == "FRAMESIZE_HQVGA" ) {
    picSnapSize = FRAMESIZE_HQVGA;
  } else if( frameSize == "FRAMESIZE_QVGA" ) {
    picSnapSize = FRAMESIZE_QVGA;
  } else if( frameSize == "FRAMESIZE_CIF" ) {
    picSnapSize = FRAMESIZE_CIF;
  } else if( frameSize == "FRAMESIZE_VGA" ) {
    picSnapSize = FRAMESIZE_VGA;
  } else if( frameSize == "FRAMESIZE_SVGA" ) {
    picSnapSize = FRAMESIZE_SVGA;
  } else if( frameSize == "FRAMESIZE_XGA" ) {
    picSnapSize = FRAMESIZE_XGA;
  } else if( frameSize == "FRAMESIZE_SXGA" ) {
    picSnapSize = FRAMESIZE_SXGA;
  } else if( frameSize == "FRAMESIZE_UXGA" ) {
    picSnapSize = FRAMESIZE_UXGA;
  } else if( frameSize == "FRAMESIZE_QXGA" ) {
    picSnapSize = FRAMESIZE_QXGA;
  } else {
    picSnapSize = FRAMESIZE_SVGA;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize( s, picSnapSize );

}

void waitForConnect ( unsigned long timeout ) {

  unsigned long timeWiFi = millis();

  while( WiFi.status() != WL_CONNECTED ) {
    if( ( millis() - timeWiFi ) > timeout )
      break;
  }

}

void initWiFi( void ) {

  wifiSTATries = 1;
  bool wifiNoSTA = false;

  WiFi.softAPdisconnect( true );
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  WiFi.mode( WIFI_STA );
  WiFi.begin( ssid, password );


// FIXME - this could be better!
/*  
  waitForConnect( 10 * 1000 );
  if( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    delay( 500 );
    // DBG_OUTPUT_PORT.println( "Connection Failed! Rebooting..." );
    // ESP.restart();
    wifiNoSTA = true;
  }
  if( wifiNoSTA ) {
    WiFi.mode( WIFI_AP );
    WiFi.begin( "AI-Cam" );
  }
 */
// FIXME - this could be better!



/* SOME ADVICE
atanisoft@atanisoft17:15
you can achieve the same with WiFi lib though :)

WiFi.onEvent([](system_event_id_t event) {
  WiFi.disconnect();
  WiFi.begin();
}, SYSTEM_EVENT_STA_LOST_IP);
 */

  WiFi.scanNetworks(true);

}

void initOTA( void ) {

  // Port defaults to 3232
  // ArduinoOTA.setPort( 3232 );

  // Hostname defaults to esp3232-[MAC]
  // add AI_CAM_SERIAL suffix
  ArduinoOTA.setHostname( "mozz-esp32-ai-1" );

  // No authentication by default
  // ArduinoOTA.setPassword( "admin" );

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash( "21232f297a57a5a743894a0e4a801fc3" );

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      DBG_OUTPUT_PORT.println("Start updating " + type);
    })
    .onEnd([]() {
      DBG_OUTPUT_PORT.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      DBG_OUTPUT_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      DBG_OUTPUT_PORT.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DBG_OUTPUT_PORT.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) DBG_OUTPUT_PORT.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) DBG_OUTPUT_PORT.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) DBG_OUTPUT_PORT.println("Receive Failed");
      else if (error == OTA_END_ERROR) DBG_OUTPUT_PORT.println("End Failed");
    });

  ArduinoOTA.begin();

}

void getNTPTime( void ) {

  DBG_OUTPUT_PORT.println( "Contacting Time Server" );
//  configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  configTime( 3600*timeZone, daySaveTime*3600, "tik.t-com.hr", "tak.t-com.hr" );
  delay( 2000 );
  tmstruct.tm_year = 0;
  getLocalTime( &tmstruct, 5000 );
  while( tmstruct.tm_year == 70 ) {
    DBG_OUTPUT_PORT.println( "NTP failed, trying again .." );
    // configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
    delay( 5000 );
    getLocalTime( &tmstruct, 5000 );
  }
  DBG_OUTPUT_PORT.printf( "Now is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year)+1900, (tmstruct.tm_mon)+1, tmstruct.tm_mday, tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec );

//  // yes, I know it can be oneliner -
//  sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
//  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_min );

}

void initSDCard( void ) {

//  if( !SD_MMC.begin() ) {
  if( !SD_MMC.begin( "/sdcard", true ) ) {
    DBG_OUTPUT_PORT.println( "SD card init failed" );
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if( cardType == CARD_NONE ){
    DBG_OUTPUT_PORT.println( "No SD card attached" );
    return;
  }

  DBG_OUTPUT_PORT.print( "SD Card Type: " );
  if(cardType == CARD_MMC){
    DBG_OUTPUT_PORT.println( "MMC" );
  } else if(cardType == CARD_SD){
    DBG_OUTPUT_PORT.println( "SDSC" );
  } else if(cardType == CARD_SDHC){
    DBG_OUTPUT_PORT.println( "SDHC" );
  } else {
    DBG_OUTPUT_PORT.println( "UNKNOWN" );
  }

  uint64_t cardSize = SD_MMC.cardSize() / ( 1024 * 1024 );
  DBG_OUTPUT_PORT.printf( "SD Card Size: %lluMB\n", cardSize );

  //  createDir(SD_MMC, "/ai-cam");
  if( !SD_MMC.mkdir( "/ai-cam" ) ) {
    DBG_OUTPUT_PORT.println( "DIR exists .." );
  }

  return;

}

void initCam( void ) {

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if( psramFound() ){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined( CAMERA_MODEL_ESP_EYE )
  pinMode( 13, INPUT_PULLUP );
  pinMode( 14, INPUT_PULLUP );
#endif

  // camera init
  esp_err_t err = esp_camera_init( &config );
  if ( err != ESP_OK ) {
    DBG_OUTPUT_PORT.printf( "Camera init failed with error 0x%x", err );
    delay( 2000 );
    ESP.restart();
//    return;
  }
  DBG_OUTPUT_PORT.println( "Camera ON!" );

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if( s->id.PID == OV3660_PID ) {
    s->set_vflip( s, 1 );       //flip it back
    s->set_brightness( s, 1 );  //up the blightness just a bit
    s->set_saturation( s, -2 ); //lower the saturation
  }
//  //drop down frame size for higher initial frame rate
//  s->set_framesize( s, FRAMESIZE_QVGA );
//  Serial.println( "Framesize QVGA" );
//  s->set_vflip( s, 1 );
//
  s->set_framesize( s, picSnapSize );
/*
typedef enum {
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    FRAMESIZE_QXGA,     // 2048*1536
    FRAMESIZE_INVALID
} framesize_t;
 */

}

void flashON( void ) {

// global settings - ignoring html on/off
  if( !FLASH_ENABLE )
    return;

// html switch on/off
  if( !flashEnable )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );
  DBG_OUTPUT_PORT.println( "Flash is ON, smile!" );

}

void flashOFF( void ) {

// turn off AI-Thinker Board Flash LED

//  if( !FLASH_ENABLE )
//    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );

// DATA1 / Flash LED - PIN4
// turn off AI-Thinker Board Flash LED
// FIXME - findout if pinMode OUTPUT makes any problems here
//  // rtc_gpio_hold_en( GPIO_NUM_4 );
//  pinMode( FLASH_LED, OUTPUT );
//  digitalWrite( FLASH_LED, LOW );

}

void doSnapPic( void ) {

  File picFile;
  String picFileDir;
  String picFileName;
  camera_fb_t * picFrameBuffer = NULL;

  tmstruct.tm_year = 0;
  getLocalTime( &tmstruct, 5000 );

  sprintf( currentDateTime, "%02d", (tmstruct.tm_mon)+1 );
  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_mday );
  picFileDir = String( "/ai-cam/" ) + currentDateTime;
  // TODO-FIXME - maybe only one subdir ??
  SD_MMC.mkdir( picFileDir ); // TODO - check error/return status
  sprintf( currentDateTime, "/%02d\0", tmstruct.tm_hour );
  picFileDir += String( currentDateTime );
  SD_MMC.mkdir( picFileDir ); // TODO - check error/return status

  // yes, I know it can be oneliner -
  sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
  sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_min );
  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_sec );
  picFileName = picFileDir + String( "/PIC-" ) + currentDateTime + String( ".jpg" ) ;
  DBG_OUTPUT_PORT.println( picFileName );

  picFile = SD_MMC.open( picFileName, FILE_WRITE );
  if( !picFile ) {
    DBG_OUTPUT_PORT.println( "error opening file for picture" );
    return;
  }

  flashON();
  delay( 50 );
  picFrameBuffer = esp_camera_fb_get();
  flashOFF();
  if( !picFrameBuffer ) {
    DBG_OUTPUT_PORT.println( "Camera capture failed" );
    return;
  }
  int picFrameLength = picFrameBuffer->len;
  DBG_OUTPUT_PORT.print( "Picture length : " );
  DBG_OUTPUT_PORT.println( picFrameLength );

  picFile.write( picFrameBuffer->buf, picFrameLength );
//  DBG_OUTPUT_PORT.println( "Wrote file .." );

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( picFrameBuffer );

  picFile.close();

  DBG_OUTPUT_PORT.printf( "Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024) );
  DBG_OUTPUT_PORT.printf( "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );

}

void setup() {

  DBG_OUTPUT_PORT.begin( 115200 );
  delay( 10 );
  DBG_OUTPUT_PORT.setDebugOutput( true );
  WiFi.printDiag(Serial); // research this
  DBG_OUTPUT_PORT.println();

  prnEspStats();

  if( !SPIFFS.begin() ) {
    DBG_OUTPUT_PORT.println( "An Error has occurred while mounting SPIFFS" );
  }

  delay( 10 );
  initWiFi();
  wifiWaitTime = millis();

  getNTPTime();

  delay( 1000 );
  initCam();
  flashOFF();

  initSDCard();

  initOTA();

  initWebServer();

  initAsyncWebServer();

  tickerSnapPic.attach( waitTime, flagSnapPicTicker );
  tickerFired = true;
  oldTickerValue = waitTime;

}

void loop() {

  ArduinoOTA.handle();
  webServer.handleClient();

  if( tickerFired ) {
    tickerFired = false;
    doSnapPic();
    fnElapsedStr( elapsedTimeString );
    DBG_OUTPUT_PORT.println( elapsedTimeString );
    if( oldTickerValue != waitTime ) {
      tickerSnapPic.detach( );
      tickerSnapPic.attach( waitTime, flagSnapPicTicker );
      oldTickerValue = waitTime;
    }
    DBG_OUTPUT_PORT.print( "Frame size set at - " );
    DBG_OUTPUT_PORT.println( foo[picSnapSize] );
  }

// FIXME - this could be better!
/*
  if( WiFi.status() != WL_CONNECTED ) {
    if( wifiWaitTime + WIFI_DISC_DELAY <= millis() ) {
      WiFi.disconnect();
      WiFi.begin();
      wifiWaitTime = millis();
    }
  }
 */
// FIXME - this could be better!

}
