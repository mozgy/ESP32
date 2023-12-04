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
#include <Base64.h>

#include "variables.h"
#include "mywebserver.h"
#include "credentials.h"
#include "camera.h"

AsyncWebServer asyncWebServer(8080);

void listDirectory( File path, AsyncWebServerRequest *request ) {

  String linkName;
  String webText;
  int numPhoto = 0;
  unsigned long atStart = millis();

  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "");
  response->addHeader("Content-Length", "CONTENT_LENGTH_UNKNOWN");
  webText = "<!DOCTYPE html><html><head><title>Mozz Cam</title>";
  webText += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'></head>";
  webText += "<body><div class='limiter'><div class='container-tableCam'><div class='wrap-tableCam'>";
  webText += "<div class='tableCam'><div class='tableCam-body'><table><tbody>";
//  request->sendChunked( webText );

  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText += "<tr><td class='co1'><a href='" + linkName + "'>" + linkName + "</a></td>";
      webText += "<td class='co2'>";
      if( linkName.endsWith( ".jpg" ) ) {
        webText += "<a href='/delete?FILENAME=" + linkName + "'>X</a>";
      } else {
        webText += "DIR"; // ToDo - add weblink to delete whole dir
      }
      webText += "</td></tr>";
      file.close();
//      request->sendChunked( webText );
      file = path.openNextFile();
//      Serial.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPhoto++;
    }
  }

  webText += "</tbody></table></div>"; // remove + if request->send is uncommented
//  webText += getHTMLTFootText( numPhoto );
  webText += "</div></div></div></div></body></html>";
  request->send( 200, "text/html", webText );

  unsigned long atEnd = millis();
  Serial.printf( "Time in listDirectory: %lu milisec\n", atEnd - atStart );
  Serial.printf( "Heap after listDirectory: %u\n", ESP.getFreeHeap() );

}

bool loadFromSDCard( AsyncWebServerRequest *request ) {

  String dataType;
  String webText;
  String path = request->url();

  File dataFile = SD_MMC.open( path.c_str() );

  if( !dataFile ) {
    return false;
  }

  if( dataFile.isDirectory() ) {
    // webText = listDirectoryAsJSON( dataFile );
    // request->send( 200, "application/json", webText );
    listDirectory( dataFile, request );
    dataFile.close();
    return true;
  }
  if( path.endsWith( ".jpg" ) ) {
    dataType = "image/jpeg";
    // request->send( SD_MMC, path.c_str(), String(), true ); // new window - download
    request->send( SD_MMC, path.c_str(), dataType );
    dataFile.close();
    return true;
  }

  return false;

}

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleRoot " );
  request->send( 200, "text/html", getHTMLRootText() );

}

void asyncHandleStatistics( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStatistics " );
  request->send( 200, "text/html", getHTMLStatisticsText() );

}

void asyncHandleStatus( AsyncWebServerRequest *request ) {

  Serial.println( " asyncHandleStatus " );
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

//  request->send( LittleFS.open( fileName.c_str() ), fileName.c_str(), "text/html" );

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
    // ToDo something with this timestamp value
  }
  doSnapPhoto();
  AsyncWebServerResponse *response = request->beginResponse( 200, "image/jpeg", photoFrame );
  response->addHeader( "Content-Disposition", "inline; filename=photo.jpg" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
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
  } else if( variable == "vflip" ) {
    err = sensor->set_vflip( sensor, valueNum );
  } else if( variable == "hmirror" ) {
    err = sensor->set_hmirror( sensor, valueNum );
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

  if( request->hasParam( "picSize" ) ) {
    AsyncWebParameter* arg2 = request->getParam( "picSize" );
    fnSetFrameSize( arg2->value() );
    Serial.print( String( arg2->value() ) );
  }
//  Serial.print( " - " );
//
//  if( request->hasParam( "timePeriod" ) ) {
//    AsyncWebParameter* arg3 = request->getParam( "timePeriod" );
////    waitTime = arg3->value().toInt(); // TODO - foolproof this
//    Serial.print( String( arg3->value() ) );
//  }
  Serial.println( " !" );

  webText = "<!doctype html><html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>Set!</body></html>";
  request->send( 200, "text/html", webText );

}

void asyncHandlePicture( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
    return;
  }

  String webText;

  // doSnapPhoto(); // ticker does the job

  // request->send( 200, "image/jpg", photoFrame ); // dangerous as photoFrame is full binary and not BASE64
  // request->send_P( 200, "image/jpg", (uint8_t*) photoFrame.c_str(), photoFrame.length() );

  webText = "<!DOCTYPE html><html><head><title>Mozz Cam</title></head>";
  webText += "<body>Neka slika .. valjda .. -<p><img src=\"data:image/jpg;base64,";
  String encodedPhoto = base64::encode( photoFrame );
  webText += encodedPhoto;
  webText += "\" alt='ESP32 Cam Photo'><p>Config - ";
  if( flashEnabled ) {
    webText += "flashOn";
  } else {
    webText += "flashOff";
  }
/*
  webText += " - ";
  if( hMirrorOn ) {
    webText += "hMirror";
  }
  webText += " - ";
  if( vFlipOn ) {
    webText += "vFlip";
  }
  webText += " - ";
  if( lensCorOn ) {
    webText += "lensCor";
  }
  webText += " - ";
  if( expCtrlOn ) {
    webText += "expCtrl";
  }
  */
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

void asyncHandleArchive( AsyncWebServerRequest *request ) {

  File photoDir;

  photoDir = SD_MMC.open( "/mozz-cam" );
  // listDirectoryAsString( photoDir );
  listDirectory( photoDir, request );
  photoDir.close();

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Not Authorized!</body></html>" );
    return;
  }

  String webText;

  AsyncWebParameter* argDelete = request->getParam( "FILENAME" );
  String fileName = argDelete->value();
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
  webText += "\nParameters: ";
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
    return;
  }

  webText = "\nNo Handler\r\n" + webText;
  request->send( 404, "text/plain", webText );
  Serial.println( webText );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, asyncHandleRoot );
  asyncWebServer.on( "/login", HTTP_GET, asyncHandleLogin );
  asyncWebServer.on( "/scan", HTTP_GET, asyncHandleScan );
  asyncWebServer.on( "/set", HTTP_GET, asyncHandleInput );
  asyncWebServer.on( "/setup", HTTP_GET, asyncHandleSetup );
  asyncWebServer.on( "/photo", HTTP_GET, asyncHandlePicture );
  asyncWebServer.on( "/stats", HTTP_GET, asyncHandleStatistics );

  asyncWebServer.on( "/fullsetup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/status", HTTP_GET, asyncHandleStatus );
  asyncWebServer.on( "/control", HTTP_GET, asyncHandleCommand );
//  asyncWebServer.on( "/xclk", HTTP_GET, asyncHandleXClk );

  asyncWebServer.on( "/capture", HTTP_GET, asyncHandleCapture );
  asyncWebServer.on( "/stream", HTTP_GET, asyncHandleNotFound );
  asyncWebServer.on( "/startLapse", HTTP_GET, asyncHandleNotFound );
  asyncWebServer.on( "/stopLapse", HTTP_GET, asyncHandleNotFound );

  asyncWebServer.on( "/delete", HTTP_GET, asyncHandleDelete );
  asyncWebServer.on( "/archive", HTTP_GET, asyncHandleArchive );

  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}