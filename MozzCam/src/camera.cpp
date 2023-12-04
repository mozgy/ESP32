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
//    return;
  }
  Serial.println( "Camera ON!" );

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if( s->id.PID == OV3660_PID ) {
    s->set_vflip( s, 1 );       // flip it back
    s->set_brightness( s, 1 );  // up the brightness just a bit
    s->set_saturation( s, -2 ); // lower the saturation
  }

/*
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }
  */

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  s->set_framesize( s, picFrameSize );

}

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
  jsonResponse += ",\"vflip\":" + String( sensor->status.vflip );
  jsonResponse += ",\"hmirror\":" + String( sensor->status.hmirror );
  jsonResponse += "}";

  Serial.println( jsonResponse );
  return jsonResponse;

}

/*
  github shameless

  sensor_t * sensor = esp_camera_sensor_get();
  sensor->set_brightness(sensor, CameraCfg.brightness);       // -2 to 2
  sensor->set_contrast(sensor, CameraCfg.contrast);           // -2 to 2
  sensor->set_saturation(sensor, CameraCfg.saturation);       // -2 to 2
  sensor->set_special_effect(sensor, 0);                      // 0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia
  sensor->set_whitebal(sensor, 1);                            // 0 = disable , 1 = enable
  sensor->set_awb_gain(sensor, 1);                            // 0 = disable , 1 = enable
  sensor->set_wb_mode(sensor, 0);                             // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  sensor->set_exposure_ctrl(sensor, CameraCfg.exposure_ctrl); // 0 = disable , 1 = enable
  sensor->set_aec2(sensor, 0);                                // 0 = disable , 1 = enable
  sensor->set_ae_level(sensor, 0);                            // -2 to 2
  sensor->set_aec_value(sensor, 300);                         // 0 to 1200
  sensor->set_gain_ctrl(sensor, 1);                           // 0 = disable , 1 = enable
  sensor->set_agc_gain(sensor, 0);                            // 0 to 30
  sensor->set_gainceiling(sensor, (gainceiling_t)0);          // 0 to 6
  sensor->set_bpc(sensor, 0);                                 // 0 = disable , 1 = enable
  sensor->set_wpc(sensor, 1);                                 // 0 = disable , 1 = enable
  sensor->set_raw_gma(sensor, 1);                             // 0 = disable , 1 = enable
  sensor->set_lenc(sensor, CameraCfg.lensc);                  // 0 = disable , 1 = enable
  sensor->set_hmirror(sensor, CameraCfg.hmirror);             // 0 = disable , 1 = enable
  sensor->set_vflip(sensor, CameraCfg.vflip);                 // 0 = disable , 1 = enable
  sensor->set_dcw(sensor, 1);                                 // 0 = disable , 1 = enable
  sensor->set_colorbar(sensor, 0);                            // 0 = disable , 1 = enable

  */
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

void flashLED( uint32_t flashONTime ) {

// html switch on/off
  if( !flashEnabled )
    return;

  flashON();
  delay( flashONTime );
  flashOFF();

}

void doSnapPhoto( void ) {

  camera_fb_t * photoFrameBuffer = NULL;
  photoFrame = "";

  flashON();
  if( flashEnabled )
    delay( 200 );
    // vTaskDelay(150 / portTICK_PERIOD_MS);
    // The LED needs to be turned on ~150ms before the call to esp_camera_fb_get()
    // or it won't be visible in the frame. A better way to do this is needed.

  int64_t capture_start = esp_timer_get_time();
  photoFrameBuffer = esp_camera_fb_get();
  flashOFF();
  if( !photoFrameBuffer ) {
    Serial.println( "Camera Capture Failed" );
    // photoFrame = load_from_dataFS( "no-pic-200x200.png" );
    return;
  }
  // if (fb->format == PIXFORMAT_JPEG) // ToDo Check, JPEG mandatory

  size_t photoFrameLength = photoFrameBuffer->len;
  for( size_t i = 0; i < photoFrameLength; i++ ) {
    photoFrame += (char) photoFrameBuffer->buf[ i ];
  }

  //  //replace this with your own function
  //  process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
 
  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( photoFrameBuffer );
  photoFrameBuffer = NULL;

  int64_t capture_end = esp_timer_get_time();
  Serial.printf("Capture Time: %uB %ums\r\n", (uint32_t)( photoFrameLength ), (uint32_t)( ( capture_end - capture_start )/1000 ) );

}

void doSnapSavePhoto( void ) {

  File photoFP;
  String photoFileDir;
  String photoFileName;
  camera_fb_t * photoFrameBuffer = NULL;
  struct tm tmstruct;

  tmstruct.tm_year = 0;
  getLocalTime( &tmstruct, 5000 );

  sprintf( currentDateTime, "%02d", (tmstruct.tm_mon)+1 );
  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_mday );
  photoFileDir = String( "/mozz-cam/" ) + currentDateTime;
  // TODO-FIXME - maybe only one subdir ??
  SD_MMC.mkdir( photoFileDir ); // TODO - check error/return status
  sprintf( currentDateTime, "/%02d\0", tmstruct.tm_hour );
  photoFileDir += String( currentDateTime );
  SD_MMC.mkdir( photoFileDir ); // TODO - check error/return status

  // yes, I know it can be oneliner -
  sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
  sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_min );
  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_sec );
  photoFileName = photoFileDir + String( "/PIC-" ) + currentDateTime + String( ".jpg" ) ;
  Serial.println( photoFileName );

  photoFP = SD_MMC.open( photoFileName, FILE_WRITE );
  if( !photoFP ) {
    Serial.println( "error opening file for picture" );
    return;
  }

  flashON();
  if( flashEnabled )
    delay( 50 );
  photoFrameBuffer = esp_camera_fb_get();
  flashOFF();
  if( !photoFrameBuffer ) {
    Serial.println( "Camera capture failed" );
    return;
  }
  int picFrameLength = photoFrameBuffer->len;
  Serial.print( "Picture length : " );
  Serial.println( picFrameLength );

  photoFP.write( photoFrameBuffer->buf, picFrameLength );
//  Serial.println( "Wrote file .." );

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( photoFrameBuffer );

  photoFP.close();

  Serial.printf( "Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024) );
  Serial.printf( "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );

}