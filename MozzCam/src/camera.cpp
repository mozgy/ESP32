//
// (c) mozgy
//
// MIT Licence
//

#include <SD_MMC.h>

#include "camera.h"
#include "camera_pins.h"

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if( config.pixel_format == PIXFORMAT_JPEG ) {
    if( psramFound() ) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
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
  }
  Serial.println( "Camera ON!" );

  sensor_t * s = esp_camera_sensor_get();

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  s->set_framesize( s, FRAMESIZE_VGA );

}

/*
void fnSetFrameSize( String frameSize ) {

  if( frameSize == "FRAMESIZE_QQVGA" ) {
    picFrameSize = FRAMESIZE_QQVGA;
  } else if( frameSize == "FRAMESIZE_QCIF" ) {
    picFrameSize = FRAMESIZE_QCIF;
  } else if( frameSize == "FRAMESIZE_HQVGA" ) {
    picFrameSize = FRAMESIZE_HQVGA;
  } else if( frameSize == "FRAMESIZE_QVGA" ) {
    picFrameSize = FRAMESIZE_QVGA;
  } else if( frameSize == "FRAMESIZE_CIF" ) {
    picFrameSize = FRAMESIZE_CIF;
  } else if( frameSize == "FRAMESIZE_VGA" ) {
    picFrameSize = FRAMESIZE_VGA;
  } else if( frameSize == "FRAMESIZE_SVGA" ) {
    picFrameSize = FRAMESIZE_SVGA;
  } else if( frameSize == "FRAMESIZE_XGA" ) {
    picFrameSize = FRAMESIZE_XGA;
  } else if( frameSize == "FRAMESIZE_SXGA" ) {
    picFrameSize = FRAMESIZE_SXGA;
  } else if( frameSize == "FRAMESIZE_UXGA" ) {
    picFrameSize = FRAMESIZE_UXGA;
  } else if( frameSize == "FRAMESIZE_QXGA" ) {
    picFrameSize = FRAMESIZE_QXGA;
  } else {
    picFrameSize = FRAMESIZE_SVGA;
  }
  sensor_t * sensor = esp_camera_sensor_get();
  sensor->set_framesize( sensor, picFrameSize );

}
  */

String getCameraStatus( void ) {

  String jsonResponse;
  sensor_t *sensor = esp_camera_sensor_get();

  jsonResponse = "{";
  jsonResponse += "\"framesize\":" + String( sensor->status.framesize );
  jsonResponse += ",\"quality\":" + String( sensor->status.quality );
  jsonResponse += ",\"brightness\":" + String( sensor->status.brightness );
  jsonResponse += ",\"contrast\":" + String( sensor->status.contrast );
  jsonResponse += ",\"saturation\":" + String( sensor->status.saturation );
  jsonResponse += ",\"sharpness\":" + String( sensor->status.sharpness );
  jsonResponse += ",\"special_effect\":" + String( sensor->status.special_effect );
  jsonResponse += ",\"wb_mode\":" + String( sensor->status.wb_mode );
  jsonResponse += ",\"awb\":" + String( sensor->status.awb );
  jsonResponse += ",\"awb_gain\":" + String( sensor->status.awb_gain );
  jsonResponse += ",\"aec\":" + String( sensor->status.aec );
  jsonResponse += ",\"aec2\":" + String( sensor->status.aec2 );
  jsonResponse += ",\"ae_level\":" + String( sensor->status.ae_level );
  jsonResponse += ",\"aec_value\":" + String( sensor->status.aec_value );
  jsonResponse += ",\"agc\":" + String( sensor->status.agc );
  jsonResponse += ",\"agc_gain\":" + String( sensor->status.agc_gain );
  jsonResponse += ",\"gainceiling\":" + String( sensor->status.gainceiling );
  jsonResponse += ",\"bpc\":" + String( sensor->status.bpc );
  jsonResponse += ",\"wpc\":" + String( sensor->status.wpc );
  jsonResponse += ",\"raw_gma\":" + String( sensor->status.raw_gma );
  jsonResponse += ",\"lenc\":" + String( sensor->status.lenc );
  jsonResponse += ",\"dcw\":" + String( sensor->status.dcw );
  jsonResponse += ",\"colorbar\":" + String( sensor->status.colorbar );
  jsonResponse += ",\"vflip\":" + String( sensor->status.vflip );
  jsonResponse += ",\"hmirror\":" + String( sensor->status.hmirror );
//  jsonResponse += ",\"pixformat\":" + String( sensor->pixformat );
//  jsonResponse += ",\"xclk\":" + String( sensor->xclk_freq_hz / 1000000 );
  jsonResponse += ",\"timelapse\":";
  jsonResponse += ( timeLapse ) ? "1" : "0";
  jsonResponse += ",\"flashled\":";
  jsonResponse += ( flashEnabled ) ? "1" : "0";
  jsonResponse += "}";

  Serial.println( jsonResponse );
  return jsonResponse;

}

/*
    // Sensor function pointers
    int  (*init_status)         (sensor_t *sensor);
    int  (*reset)               (sensor_t *sensor); // Reset the configuration of the sensor, and return ESP_OK if reset is successful
    int  (*set_pixformat)       (sensor_t *sensor, pixformat_t pixformat);
    int  (*set_framesize)       (sensor_t *sensor, framesize_t framesize);
    int  (*set_contrast)        (sensor_t *sensor, int level);
    int  (*set_brightness)      (sensor_t *sensor, int level);
    int  (*set_saturation)      (sensor_t *sensor, int level);
    int  (*set_sharpness)       (sensor_t *sensor, int level);
    int  (*set_denoise)         (sensor_t *sensor, int level);
    int  (*set_gainceiling)     (sensor_t *sensor, gainceiling_t gainceiling);
    int  (*set_quality)         (sensor_t *sensor, int quality);
    int  (*set_colorbar)        (sensor_t *sensor, int enable);
    int  (*set_whitebal)        (sensor_t *sensor, int enable);
    int  (*set_gain_ctrl)       (sensor_t *sensor, int enable);
    int  (*set_exposure_ctrl)   (sensor_t *sensor, int enable);
    int  (*set_hmirror)         (sensor_t *sensor, int enable);
    int  (*set_vflip)           (sensor_t *sensor, int enable);

    int  (*set_aec2)            (sensor_t *sensor, int enable);
    int  (*set_awb_gain)        (sensor_t *sensor, int enable);
    int  (*set_agc_gain)        (sensor_t *sensor, int gain);
    int  (*set_aec_value)       (sensor_t *sensor, int gain);

    int  (*set_special_effect)  (sensor_t *sensor, int effect);
    int  (*set_wb_mode)         (sensor_t *sensor, int mode);
    int  (*set_ae_level)        (sensor_t *sensor, int level);

    int  (*set_dcw)             (sensor_t *sensor, int enable);
    int  (*set_bpc)             (sensor_t *sensor, int enable);
    int  (*set_wpc)             (sensor_t *sensor, int enable);

    int  (*set_raw_gma)         (sensor_t *sensor, int enable);
    int  (*set_lenc)            (sensor_t *sensor, int enable);

    int  (*get_reg)             (sensor_t *sensor, int reg, int mask);
    int  (*set_reg)             (sensor_t *sensor, int reg, int mask, int value);
    int  (*set_res_raw)         (sensor_t *sensor, int startX, int startY, int endX, int endY, int offsetX, int offsetY, int totalX, int totalY, int outputX, int outputY, bool scale, bool binning);
    int  (*set_pll)             (sensor_t *sensor, int bypass, int mul, int sys, int root, int pre, int seld5, int pclken, int pclk);
    int  (*set_xclk)            (sensor_t *sensor, int timer, int xclk);
  */

void flashON( void ) {

// global settings - ignoring html on/off
//  if( !FLASH_ENABLE )
//    return;

// html switch on/off
  if( !flashEnabled )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );
//  Serial.println( "Flash is ON, smile!" );

}

void flashON( bool forcedFlash ) {

  if( !forcedFlash )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );

}

void flashOFF( void ) {

// global settings - ignoring html on/off
//  if( !FLASH_ENABLE )
//    return;

// html switch on/off
  if( !flashEnabled )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );

// DATA1 / Flash LED - PIN4
// turn off AI-Thinker Board Flash LED
// FIXME - findout if pinMode OUTPUT makes any problems here
//  // rtc_gpio_hold_en( GPIO_NUM_4 );
//  pinMode( FLASH_LED, OUTPUT );
//  digitalWrite( FLASH_LED, LOW );

}

void flashOFF( bool forcedFlash ) {

  if( !forcedFlash )
    return;

  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );

}

void flashLED( uint32_t flashONTime ) {

// html switch on/off
  if( !flashEnabled )
    return;

  flashON();
  delay( flashONTime );
  flashOFF();

}

void flashLED( uint32_t flashONTime, bool forcedFlash ) {

  if( !forcedFlash )
    return;

  flashON( true );
  delay( flashONTime );
  flashOFF( true );

}

// void doSnapPhoto( void ) {
void doSnapSavePhoto( void ) {

  File photoFP;
  String photoFileDir;
  String photoFileName;
  camera_fb_t * photoFrameBuffer = NULL;
  struct tm tmstruct;
  tmstruct.tm_year = 0;
  photoFrame = "";

  if( timeLapse && SDCardOK ) {
    String photoFileDir;
    String photoFileName;

    getLocalTime( &tmstruct, 5000 );

    photoFileDir = String( "/mozz-cam/" );  // /mozz-cam/
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      Serial.println( "MKDIR Failed!" );
    }
    sprintf( currentDateTime, "%04d\0", (tmstruct.tm_year)+1900 );
    photoFileDir += String( currentDateTime );  // /mozz-cam/YYYY
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      Serial.println( "MKDIR Failed!" );
    }
    sprintf( currentDateTime, "/%02d%02d\0", (tmstruct.tm_mon)+1, tmstruct.tm_mday );
    photoFileDir += String( currentDateTime );  // /mozz-cam/YYYY/MMDD
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      Serial.println( "MKDIR Failed!" );
    }
    sprintf( currentDateTime, "/%02d\0", tmstruct.tm_hour );
    photoFileDir += String( currentDateTime );
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      Serial.println( "MKDIR Failed!" );
    }

    // yes, I know it can be oneliner -
    sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
    sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
    sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
    sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
    sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_min );
    sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_sec );
    photoFileName = photoFileDir + String( "/photo-" ) + currentDateTime + String( ".jpg" ) ;
    Serial.println( photoFileName );

    photoFP = SD_MMC.open( photoFileName, FILE_WRITE );
    if( !photoFP ) {
      Serial.println( "SD Card file open for write error" );
      return;
    }
  }

  flashON();
  if( flashEnabled )
    delay( 200 );
    // vTaskDelay(150 / portTICK_PERIOD_MS);
    // The LED needs to be turned on ~150ms before the call to esp_camera_fb_get()
    // or it won't be visible in the frame. A better way to do this is needed.

  int64_t capture_start = esp_timer_get_time();
  photoFrameBuffer = esp_camera_fb_get();
//  if( !photoFrameBuffer )
//    photoFrameBuffer = esp_camera_fb_get(); // second time the charm ??
  flashOFF();
  if( !photoFrameBuffer ) {
    Serial.println( "Camera Capture Failed" );
    // photoFrame = load_from_dataFS( "no-pic-200x200.png" );
    return;
  }
  // if (fb->format == PIXFORMAT_JPEG) // ToDo Check, JPEG mandatory

  size_t photoFrameLength = photoFrameBuffer->len;
  // Serial.print( "Picture length : " );
  // Serial.println( photoFrameLength );
  for( size_t i = 0; i < photoFrameLength; i++ ) {
    photoFrame += (char) photoFrameBuffer->buf[ i ];
  }

  //  //replace this with your own function
  //  process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
  if( timeLapse && SDCardOK ) {
    photoFP.write( photoFrameBuffer->buf, photoFrameLength );
    Serial.printf( "Time Lapse ON - Wrote File - Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );
  }

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( photoFrameBuffer );
  photoFrameBuffer = NULL;

  if( timeLapse && SDCardOK ) {
    photoFP.close();
  }

  int64_t capture_end = esp_timer_get_time();
  Serial.printf("Capture Time: %uB %ums\r\n", (uint32_t)( photoFrameLength ), (uint32_t)( ( capture_end - capture_start )/1000 ) );
//  Serial.printf( "Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024) );
//  Serial.printf( "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );

}
