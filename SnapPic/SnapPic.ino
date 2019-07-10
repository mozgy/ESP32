//
// (c) mozgy
//
// MIT Licence
//

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <FS.h>
#include <SD_MMC.h>
// #include "driver/rtc_io.h"

#define SW_VERSION "1.01.18"

#define AI_CAM_SERIAL "3"

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
// DATA1 / Flash LED - PIN4
#define FLASH_LED 4
#define FLASH_ENABLE false

#include "credentials.h"

long timeZone = 1;
byte daySaveTime = 1;
struct tm tmstruct;

char elapsedTimeString[40];
char currentDateTime[17];

WebServer webServer(80);

AsyncWebServer asyncWebServer(8080);

Ticker tickerSnapPic;
#define WAITTIME 60
boolean tickerFired;

void flagSnapPicTicker( void ) {
  tickerFired = true;
}

void prnEspStats( void ) {

  uint64_t chipid;

  Serial.println();
  Serial.printf( "Sketch SW version: %s\n", SW_VERSION );
  Serial.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  Serial.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  Serial.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  Serial.printf( "Chip Revision: %u\n", ESP.getChipRevision() );
  Serial.printf( "SDK: %s\n", ESP.getSdkVersion() );
//  Serial.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  Serial.printf( "Flash Size: %u\n", ESP.getFlashChipSize() );
  Serial.printf( "Flash Speed: %u\n", ESP.getFlashChipSpeed() );
//  Serial.printf( "Chip ID: %u\n", ESP.getChipId() );
  chipid=ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf( "ESP32 Chip ID = %04X", (uint16_t)(chipid>>32) ); //print High 2 bytes
  Serial.printf( "%08X\n", (uint32_t)chipid );                     //print Low 4bytes.
//  Serial.printf( "Vcc: %u\n", ESP.getVcc() );
  Serial.println();

}

void ElapsedStr( char *str ) {

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

void waitForConnect ( unsigned long timeout ) {

  unsigned long timeWiFi = millis();

  while( WiFi.status() != WL_CONNECTED ) {
    if( ( millis() - timeWiFi ) > timeout )
      break;
  }

}

void initWiFi( void ) {

  WiFi.softAPdisconnect( true );
  WiFi.mode( WIFI_STA );
  WiFi.begin( ssid, password );

// FIXME - this could be better!
  waitForConnect( 10 * 1000 );
  while( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println( "Connection Failed! Rebooting..." );
    delay( 5000 );
    ESP.restart();
  }
// FIXME - this could be better!

  Serial.println( "WiFi connected" );

/* SOME ADVICE
atanisoft@atanisoft17:15
you can achieve the same with WiFi lib though :)

WiFi.onEvent([](system_event_id_t event) {
  WiFi.disconnect();
  WiFi.begin();
}, SYSTEM_EVENT_STA_LOST_IP);
 */

}

void initOTA( void ) {

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // add AI_CAM_SERIAL suffix
  ArduinoOTA.setHostname("mozz-esp32-ai-3");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

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

  Serial.println( "Contacting Time Server" );
//  configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  configTime( 3600*timeZone, daySaveTime*3600, "tik.t-com.hr", "tak.t-com.hr" );
  delay( 2000 );
  tmstruct.tm_year = 0;
  getLocalTime( &tmstruct, 5000 );
  while( tmstruct.tm_year == 70 ) {
    Serial.println( "NTP failed, trying again .." );
    // configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
    delay( 5000 );
    getLocalTime( &tmstruct, 5000 );
  }
  Serial.printf( "Now is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year)+1900, (tmstruct.tm_mon)+1, tmstruct.tm_mday, tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec );

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
    Serial.println( "SD card init failed" );
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

  //  createDir(SD_MMC, "/ai-cam");
  if( !SD_MMC.mkdir( "/ai-cam" ) ) {
    Serial.println( "DIR exists .." );
  }

  return;

// FIXME - put SD_MMC stuff in separate file !?
//  listDir(SD_MMC, "/", 0);
//  removeDir(SD_MMC, "/mydir");
//  createDir(SD_MMC, "/mydir");
//  deleteFile(SD_MMC, "/hello.txt");
//  writeFile(SD_MMC, "/hello.txt", "Hello ");
//  appendFile(SD_MMC, "/hello.txt", "World!\n");
//  listDir(SD_MMC, "/", 0);

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
    Serial.printf( "Camera init failed with error 0x%x", err );
    delay( 2000 );
    ESP.restart();
//    return;
  }
  Serial.println( "Camera ON!" );

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
//
//  s->set_vflip( s, 1 );
//  s->set_framesize( s, FRAMESIZE_SVGA );
//  Serial.println( "Framesize SVGA - 800x600" );
  s->set_framesize( s, FRAMESIZE_XGA );
  Serial.println( "Framesize XGA - 1024x768" );

}

void flashON( void ) {

  if( !FLASH_ENABLE )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );

}

void flashOFF( void ) {

//  if( !FLASH_ENABLE )
//    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );

// DATA1 / Flash LED - PIN4
// turn off AI-Thinker Board Flash LED
// FIXME - findout if pinMode OUTPUT makes any problems here
//  pinMode( 4, OUTPUT );
//  digitalWrite( 4, LOW );
//  // rtc_gpio_hold_en( GPIO_NUM_4 );
//  // FIXME - findout if pinMode OUTPUT makes any problems here
//  pinMode( FLASH_LED, OUTPUT );
//  // turn off AI-Thinker Board Flash LED
//  digitalWrite( FLASH_LED, LOW );

}

void doSnapPic( void ) {

  File picFileP;
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
  Serial.println( picFileName );

  picFileP = SD_MMC.open( picFileName, FILE_WRITE );
  if( !picFileP ) {
    Serial.println( "error opening file for picture" );
  }

  flashON();
  delay( 50 );
  picFrameBuffer = esp_camera_fb_get();
  flashOFF();
  if( !picFrameBuffer ) {
    Serial.println( "Camera capture failed" );
    return;
  }
  int picFrameLength = picFrameBuffer->len;
  Serial.print( "Picture length : " );
  Serial.println( picFrameLength );

  picFileP.write( picFrameBuffer->buf, picFrameLength );
//  Serial.println( "Wrote file .." );

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( picFrameBuffer );

  picFileP.close();

  Serial.printf( "Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024) );
  Serial.printf( "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );

}

void setup() {

  Serial.begin( 115200 );
  delay( 10 );
  Serial.setDebugOutput( true );
  WiFi.printDiag(Serial); // research this
  Serial.println();

  prnEspStats();

  delay( 10 );
  initWiFi();

  getNTPTime();

  delay( 1000 );
  initCam();
  flashOFF();

  initSDCard();

  initOTA();

  initWebServer();

  initAsyncWebServer();

  tickerSnapPic.attach( WAITTIME, flagSnapPicTicker );
  tickerFired = true;

}

void loop() {

  ArduinoOTA.handle();
  webServer.handleClient();

  if( tickerFired ) {
    tickerFired = false;
    doSnapPic();
    ElapsedStr( elapsedTimeString );
    Serial.println( elapsedTimeString );
  }

  if( WiFi.status() != WL_CONNECTED ) {
    WiFi.disconnect();
    WiFi.begin();
  }

}
