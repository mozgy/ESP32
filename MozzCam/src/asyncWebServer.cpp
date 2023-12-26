//
// (c) mozgy
//
// MIT Licence
//

#include <stddef.h>
#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <base64.h>

#include "variables.h"
#include "mywebserver.h"
#include "credentials.h"
#include "camera.h"

AsyncWebServer asyncWebServer(8080);

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleRoot " );
  request->send( 200, "text/html", getHTMLRootText() );

}

void asyncHandleStatistics( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStatistics " );
  request->send( 200, "text/html", getHTMLStatisticsText() );

}

void asyncHandleStatus( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStatus " );
  AsyncWebServerResponse *response = request->beginResponse( 200, "application/json", getCameraStatus() );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleSetup( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  // Serial.println( " asyncHandleSetup " );
  request->send( 200, "text/html", getHTMLSetupText() );

}

void asyncHandleFullSetup( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  Serial.println( " asyncHandleFullSetup " );

  String fileName = "/esp32setup.html";

  // request->send( LittleFS.open( fileName.c_str() ), fileName.c_str(), "text/html" );

  File filePointer = LittleFS.open( fileName.c_str(), "r" );
  if( filePointer ) {
    if( filePointer.available() ) {
      String stringHTMLFullSetupText = filePointer.readString();
      request->send( 200, "text/html", stringHTMLFullSetupText );
    }
    filePointer.close();
    return;
  }
  request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>HTML File Not Found!</body></html>" );

}

void asyncHandleLogin( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) )
    return request->requestAuthentication();  // Hm? Double-check this return ..
  request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Login Success!</body></html>" );

}

void asyncHandleCapture( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleCapture " );
  // http://192.168.1.164:8080/capture?_cb=1701038417082
  String value = "";
  if( request->hasParam( "_cb" ) ) {
    AsyncWebParameter* arg = request->getParam( "_cb" );
    value = arg->value().c_str();
  }
  if( !timeLapse )  // if on timelapse mode just take last photo taken
    doSnapSavePhoto();
  AsyncWebServerResponse *response = request->beginResponse( 200, "image/jpeg", photoFrame );
  response->addHeader( "Content-Disposition", "inline; filename=photo.jpg" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  response->addHeader( "X-Timestamp", value );
/* this has to be inside capture {}
  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  response->addHeader( "X-Timestamp", (const char *)ts );
  */
  request->send( response );

}

void asyncHandleCommand( AsyncWebServerRequest *request ) {

//  const query = `${baseHost}/control?var=${el.id}&val=${value}`
  String variable = "";
  String value = "";

  if( request->hasParam( "var" ) ) {
    AsyncWebParameter* arg = request->getParam( "var" );
    variable = arg->value().c_str();
    if( request->hasParam( "val" ) ) {
      AsyncWebParameter* arg = request->getParam( "val" );
      value = arg->value();
    }
  } else {
    request->send( 500, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Wrong Input Parameter!</body></html>" );
    return;
  }

  Serial.printf( " asyncHandleCommand - %s - %s\n", variable, value );

  sensor_t *sensor = esp_camera_sensor_get();
  int err = 0;
  int valueNum = value.toInt();

  if( variable == "framesize" ) {
    if( sensor->pixformat == PIXFORMAT_JPEG ) {
      err = sensor->set_framesize( sensor, (framesize_t)valueNum );
    }
  } else if( variable == "quality" ) {
    err = sensor->set_quality( sensor, valueNum );
  } else if( variable == "brightness" ) {
    err = sensor->set_brightness( sensor, valueNum );
  } else if( variable == "contrast" ) {
    err = sensor->set_contrast( sensor, valueNum );
  } else if( variable == "saturation" ) {
    err = sensor->set_saturation( sensor, valueNum );
  } else if( variable == "sharpness" ) {
    err = sensor->set_sharpness( sensor, valueNum );
  } else if( variable == "special_effect" ) {
    err = sensor->set_special_effect( sensor, valueNum );
  } else if( variable == "wb_mode" ) {
    err = sensor->set_wb_mode( sensor, valueNum );
  } else if( variable == "awb" ) {
    err = sensor->set_whitebal( sensor, valueNum );
  } else if( variable == "vflip" ) {
    err = sensor->set_vflip( sensor, valueNum );
  } else if( variable == "hmirror" ) {
    err = sensor->set_hmirror( sensor, valueNum );
  } else if( variable == "timelapse" ) {
    timeLapse = !timeLapse;
  } else if( variable == "flashled" ) {
#if defined( CAMERA_MODEL_AI_THINKER )
    flashEnabled = !flashEnabled;
#else
    flashEnabled = false;
#endif
  } else {
    err = -1;
  }

  if( err ) {
    request->send( 500, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Wrong Input Parameter!</body></html>" );
    return;
  }

  AsyncWebServerResponse *response = request->beginResponse( 200, "", "" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleXClk( AsyncWebServerRequest *request ) {

//  fetchUrl(`${baseHost}/xclk?xclk=${xclk}`, cb);
  sensor_t *sensor = esp_camera_sensor_get();
  int err = 0;
  String value = "";

  if( request->hasParam( "xclk" ) ) {
    AsyncWebParameter* arg = request->getParam( "xclk" );
    value = arg->value();
    int valueNum = value.toInt();
    log_i( "Set XCLK: %d MHz", valueNum );
    int res = sensor->set_xclk( sensor, LEDC_TIMER_0, valueNum );
    if( err ) {
      request->send( 500, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Wrong Input Parameter!</body></html>" );
    }
  }
  AsyncWebServerResponse *response = request->beginResponse( 200, "", "" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

// OLD format code - ToDo migrate to js
void asyncHandleInput( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  String webText;
  flashEnabled = false;

  Serial.print( "Got arguments - " );

  if( request->hasParam( "flashswitch" ) ) {
    AsyncWebParameter* arg1 = request->getParam( "flashswitch" );
    if( arg1->value() == "flashOn" )
      flashEnabled = true;
    Serial.print( String( arg1->value().c_str() ) );
  }
  Serial.print( " - " );

//  if( request->hasParam( "picSize" ) ) {
//    AsyncWebParameter* arg2 = request->getParam( "picSize" );
//    fnSetFrameSize( arg2->value() );
//    Serial.print( String( arg2->value() ) );
//  }
//  Serial.print( " - " );
//
//  if( request->hasParam( "timePeriod" ) ) {
//    AsyncWebParameter* arg3 = request->getParam( "timePeriod" );
////    waitTime = arg3->value().toInt(); // TODO - foolproof this
//    Serial.print( String( arg3->value() ) );
//  }
  Serial.println( " !" );

  webText = "<!doctype html><html><head><meta http-equiv='refresh' content='2; URL=/'></head><body>Set!</body></html>";
  request->send( 200, "text/html", webText );

}

// OLD format code - ToDo migrate to js
void asyncHandlePicture( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
    return;
  }

  String webText;

  // doSnapPhoto(); // ticker does the job

  // request->send( 200, "image/jpg", photoFrame ); // dangerous as photoFrame is full binary and not BASE64
  // request->send_P( 200, "image/jpg", (uint8_t*) photoFrame.c_str(), photoFrame.length() );

  webText = "<!doctype html><html><head><title>Mozz Cam</title></head>";
  webText += "<body>Neka slika .. valjda .. -<p><img src=\"data:image/jpg;base64,";
  String encodedPhoto = base64::encode( photoFrame );
  webText += encodedPhoto;
  webText += "\" alt='ESP32 Cam Photo'><p>Config - ";
  if( flashEnabled ) {
    webText += "flashOn";
  } else {
    webText += "flashOff";
  }
//  webText += String( picFrameSize );
  webText += " !</body></html>";   // width="128" height="128"
  Serial.print( "Captured len - " );
  Serial.print( photoFrame.length() );
  Serial.print( " Encoded len - " );
  Serial.println( encodedPhoto.length() );
  request->send( 200, "text/html", webText );

}

void asyncHandleScan( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks( true );
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
//      json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}";
    }
    WiFi.scanDelete();
    if( WiFi.scanComplete() == -2 ){
      WiFi.scanNetworks( true );
    }
  }
  json += "]";
  request->send( 200, "application/json", json );
  json = String();

}
void asyncHandleESPReset( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  Serial.println( "Restarting in 10 seconds" );
  request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='30; URL=/'></head><body>ESP Restart!</body></html>" );
  delay( 10000 );
  WiFi.disconnect();
  ESP.restart();

}

void asyncHandleArchive( AsyncWebServerRequest *request ) {

  listDirectory( "/mozz-cam", request );

}

// FIXME - NOT WORKY !!
void asyncHandleSDCardRemount( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  SD_MMC.end();
  delay( 1000 );
  initSDCard();
  request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>SD Card Remount!</body></html>" );

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  String webText;

  AsyncWebParameter* argDelete = request->getParam( "filename" );
  String fileName = argDelete->value();

  Serial.printf( " asyncHandleDelete - %s\n", fileName );
  webText = "ToDelete - ";
  webText += fileName;
  request->send( 200, "text/plain", webText );
  return;

//  deleteFile( SD_MMC, fileName );
  if( SD_MMC.remove( fileName.c_str() ) ) {
    webText = "Deleted - " + String( fileName );
    Serial.print( "Deleted - " );
    Serial.println( fileName );
  } else {
    webText = "Cannot delete - " + String( fileName );
    Serial.print( "Cannot delete - " );
    Serial.println( fileName );
  }
  request->send( 200, "text/plain", webText );

}

/*
void asyncHandleStartAP( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
    return;
  }
  WiFi.softAP( "MozzCam" );
  IPAddress IP = WiFi.softAPIP();
  Serial.print( "AP IP address: " );
  Serial.println( IP );
  request->send( 200, "text/plain", "AP Started!" );

}
*/

void asyncHandleNotFound( AsyncWebServerRequest *request ) {

  String path = request->url();
  String dataType = "text/plain";
  String webText;

  int lastSlash = path.lastIndexOf( '/' );
  String fileName = path.substring( lastSlash, path.length() );

  webText = "URI: ";
  webText += request->url();
  webText += "\nMethod: ";
  webText += ( request->method() == HTTP_GET ) ? "GET" : "POST";
  webText += ", Parameters: ";
  webText += request->params();
  webText += "\n";
  for( uint8_t i = 0 ; i < request->params(); i++ ) {
    AsyncWebParameter* p = request->getParam( i );
    webText += String( p->name().c_str() ) + " : " + String( p->value().c_str() ) + "\r\n";
  }

  Serial.print( "Basename - " );
  Serial.println( fileName );
  Serial.println( webText );

  bool fileLocalFS = false;
  if( fileName.endsWith( ".css" ) ) {
    dataType = "text/css";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".html" ) ) {
    dataType = "text/html";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".ico" ) ) {
    dataType = "image/png";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".js" ) ) {
    dataType = "aplication/javascript";
    fileLocalFS = true;
  }
  if( fileLocalFS ) {
    request->send( LittleFS.open( fileName.c_str() ), fileName.c_str(), dataType );
    // File fp = LittleFS.open( fileName.c_str() ); // FIXME - NO WORKY
    // request->send( fp, fileName.c_str(), dataType );
    // fp.close();
    return;
  } else {
    if( loadFromSDCard( request ) ) {
      return;
    }
  }

  webText = "\nNo Handler\r\n" + webText;
  request->send( 404, "text/plain", webText );
  Serial.println( webText );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, asyncHandleRoot );
  asyncWebServer.on( "/login", HTTP_GET, asyncHandleLogin );
  asyncWebServer.on( "/setup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/photo", HTTP_GET, asyncHandlePicture );
  asyncWebServer.on( "/stats", HTTP_GET, asyncHandleStatistics );

  asyncWebServer.on( "/mozzsetup", HTTP_GET, asyncHandleSetup );
  asyncWebServer.on( "/set", HTTP_GET, asyncHandleInput );
  asyncWebServer.on( "/fullsetup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/status", HTTP_GET, asyncHandleStatus );

  asyncWebServer.on( "/control", HTTP_GET, asyncHandleCommand );
  asyncWebServer.on( "/capture", HTTP_GET, asyncHandleCapture );
  asyncWebServer.on( "/stream", HTTP_GET, asyncHandleNotFound );  // TODO

  asyncWebServer.on( "/delete", HTTP_GET, asyncHandleDelete );    // TODO
  asyncWebServer.on( "/archive", HTTP_GET, asyncHandleArchive );
  asyncWebServer.on( "/sdcard", HTTP_GET, asyncHandleSDCardRemount );

  asyncWebServer.on( "/scan", HTTP_GET, asyncHandleScan );
  asyncWebServer.on( "/espReset", HTTP_GET, asyncHandleESPReset );

  // asyncWebServer.on( "/xclk", HTTP_GET, asyncHandleXClk );

  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}
