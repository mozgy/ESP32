//
// (c) mozgy
//
// MIT Licence
//

#include <Arduino.h>

// #include "driver/rtc_io.h"

#include <LittleFS.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Ticker.h>
#include <ArduinoOTA.h>

#include "credentials.h"
#include "variables.h"
#include "camera.h"
#include "mywebserver.h"
#include "mywifi.h"

long timeZone = 1;
byte daySaveTime = 1;
struct tm startTime;

char elapsedTimeString[40];
char currentDateTime[17];

String photoFrame;

bool flashEnabled = FLASH_ENABLED;
bool timeLapse = TIME_LAPSE_MODE;
bool SDCardOK;

Ticker tickerSnapPic;
boolean tickerFired;
int tickerMissed;
int waitTime = 60;
int oldTickerValue;

void flagSnapPicTicker( void ) {
  tickerFired = true;
  tickerMissed++;
}

void prnEspStats( void ) {

  uint64_t chipid;

  Serial.println();
  Serial.printf( "Sketch SW version: %s\n", SW_VERSION );
  Serial.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  Serial.printf( "Sketch MD5: %u\n", ESP.getSketchMD5() );
  Serial.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  Serial.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  Serial.printf( "Chip Revision: %u\n", ESP.getChipRevision() );
  Serial.printf( "SDK: %s\n", ESP.getSdkVersion() );
//  Serial.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  Serial.printf( "Flash Size: %u\n", ESP.getFlashChipSize() );
  Serial.printf( "Flash Speed: %u\n", ESP.getFlashChipSpeed() );
  Serial.printf( "PSRAM Size: %u\n", ESP.getPsramSize() );
//  Serial.printf( "Chip ID: %u\n", ESP.getChipId() );
  chipid=ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf( "ESP32 Chip ID = %04X", (uint16_t)(chipid>>32) ); //print High 2 bytes
  Serial.printf( "%08X\n", (uint32_t)chipid );                     //print Low 4bytes.
//  Serial.printf( "Vcc: %u\n", ESP.getVcc() );
  Serial.println();

}

void fnElapsedStr( char *str ) {

  unsigned long sec;
  int minute, hour, day;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  day = hour / 24;
  if ( day > 0 ) {
    hour = hour % 24;
  }
  sprintf( str, "Elapsed " );
  if ( day > 0 ) {
    sprintf( str, "%s%d:", str, day );
  }
  if ( hour != 0 ) {
    sprintf( str, "%s%2d:", str, hour );
//  } else {
//    sprintf( str, "%s   ", str );
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

void initOTA( void ) {

  // Port defaults to 3232
  // ArduinoOTA.setPort( 3232 );

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname( "MozzCam" );

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
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

}

void getNTPTime( void ) {

  Serial.print( "Contacting Time Server - " );
//  configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  configTime( 3600*timeZone, daySaveTime*3600, "tik.t-com.hr", "tak.t-com.hr" );
  delay( 2000 );
  startTime.tm_year = 0;
  getLocalTime( &startTime, 5000 );
  while( startTime.tm_year == 70 ) {
    Serial.print( "NTP failed, trying again .. " );
    delay( 5000 );
    getLocalTime( &startTime, 5000 );
  }
  Serial.printf( "Local Time : %d-%02d-%02d %02d:%02d:%02d\n", (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );

//  // yes, I know it can be oneliner -
//  sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
//  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_min );

}

void initSDCard( void ) {

// if defined( CAMERA_MODEL_XIAO_ESP32S3 )
// if( !SD.begin( XIAO_ESP32S3_SDCS_PIN ) { // ehm??
//    bool setPins(int clk, int cmd, int d0);
//    bool setPins(int clk, int cmd, int d0, int d1, int d2, int d3);

  SDCardOK = true;
#ifdef CAMERA_MODEL_AI_THINKER
//  if( !SD_MMC.begin() ) { // fast 4bit mode
  if( !SD_MMC.begin( "/sdcard", true ) ) { // slow 1bit mode
//  if( !SD_MMC.begin( "/sdcard", true, true ) ) { // slow 1bit mode, format card
#else
 #ifdef CAMERA_MODEL_XIAO_ESP32S3
// this FUQS up Camera get picture ..
// pins configured for SD card on this camera board
// WHICH one of those is XIAO Sense ???

//  #define SD_MMC_CLK 7
//  #define SD_MMC_CMD 9
//  #define SD_MMC_D0 8

//  #define SD_MMC_D0   2
//  #define SD_MMC_CLK  14
//  #define SD_MMC_CMD  15

// #define SD_MMC_CMD 38
// #define SD_MMC_CLK 39
// #define SD_MMC_D0  40

//  SD_MMC.setPins( SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0 );
 #endif
// bool SDMMCFS::begin(const char * mountpoint, bool mode1bit, bool format_if_mount_failed, int sdmmc_frequency, uint8_t maxOpenFiles)
  // if( !SD_MMC.begin( "/sdcard", true, false, SDMMC_FREQ_DEFAULT, 5 ) ) {
  // if( !SD_MMC.begin( "/sdcard", true, false, 20000 ) ) {
  if( !SD_MMC.begin() ) {
#endif
    Serial.println( "SD card init failed" );
    SDCardOK = false;
    timeLapse = false;
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if( cardType == CARD_NONE ){
    Serial.println( "No SD card attached" );
    return;
  }

  Serial.print( "SD Card Type: " );
  if(cardType == CARD_MMC){
    Serial.println( "MMC" );
  } else if(cardType == CARD_SD){
    Serial.println( "SDSC" );
  } else if(cardType == CARD_SDHC){
    Serial.println( "SDHC" );
  } else {
    Serial.println( "UNKNOWN" );
  }

  uint64_t cardSize = SD_MMC.cardSize() / ( 1024 * 1024 );
  Serial.printf( "SD Card Size: %lluMB\n", cardSize );

  if( !SD_MMC.mkdir( "/mozz-cam" ) ) {
    Serial.println( "DIR exists .." );
  }

  return;

}

void setup() {

  Serial.begin( 115200 );
  while( !Serial );
  delay( 100 );
  Serial.println( "Setup Start!" );
  Serial.setDebugOutput( true );
  // WiFi.printDiag(Serial); // research this
  Serial.println();

  // set these three lines above BEFORE AND AFTER the call to esp_wifi_init
  // esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  // esp_wifi_internal_set_log_level(WIFI_LOG_VERBOSE);
  // esp_wifi_internal_set_log_mod(WIFI_LOG_MODULE_ALL, WIFI_LOG_SUBMODULE_ALL, true);

  prnEspStats();

  if( !LittleFS.begin( true ) ) {
    Serial.println( "Formatting LittleFS" );
    if( !LittleFS.begin( ) ) {
      Serial.println( "An Error has occurred while mounting LittleFS" );
    }
  }

  delay( 10 );
  initWiFi();
  wifiWaitTime = millis();
  getNTPTime();
  Serial.println( WiFi.localIP() );

  delay( 10 );
  // [E][SD_MMC.cpp:132] begin(): Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.
  initSDCard( );

#ifndef CAMERA_MODEL_AI_THINKER
  // using LED only on ESP32-CAM (originaly made by AI-Thinker)
  flashEnabled = false;
#endif

#ifdef HAVE_CAMERA
  delay( 10 );
  initCam();
#endif

  delay( 10 );
  initOTA();
  initAsyncWebServer();

  tickerMissed = 0;
  tickerSnapPic.attach( waitTime, flagSnapPicTicker );
  tickerFired = true;
  oldTickerValue = waitTime;

}

void loop() {

  ArduinoOTA.handle();  // FIXME - not working ATM

  if( tickerFired ) {
    int wifiStatus;
    tickerFired = false;
#ifdef HAVE_CAMERA
    doSnapSavePhoto();
#endif
    wifiStatus = WiFi.status();
    Serial.println( get_wifi_status( wifiStatus ) );
    Serial.print( "(RSSI): " );
    Serial.print( WiFi.RSSI() );
    Serial.print( " dBm, MAC=" );
    Serial.print( WiFi.macAddress() );
    if( WiFi.status() == WL_CONNECTED ) {
      Serial.print( ", IP=" );
      Serial.print( WiFi.localIP() );
    }
    Serial.println();
    fnElapsedStr( elapsedTimeString );
    Serial.printf( "%s - Startup Time : %d-%02d-%02d %02d:%02d:%02d\n", elapsedTimeString, (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );
    if( oldTickerValue != waitTime ) {
      tickerSnapPic.detach( );
      tickerSnapPic.attach( waitTime, flagSnapPicTicker );
      oldTickerValue = waitTime;
    }
    if( tickerMissed > 1 ) {
      Serial.printf( "Missed %d tickers\n", tickerMissed - 1 );
    }
    tickerMissed = 0;
  }

}
