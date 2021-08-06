//
// (c) mozgy
//
// MIT Licence
//

#include "variables.h"

/// AsyncWebServer Definitions
#ifdef USE_ASYNC_WEBSERVER

void listDirectory( File path, AsyncWebServerRequest *request ) {

  String linkName;
  String webText;
  int numPic = 0;
  unsigned long atStart = millis();

  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "");
  response->addHeader("Content-Length", "CONTENT_LENGTH_UNKNOWN");
  webText = "<!DOCTYPE html><html><head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
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
        webText += "DIR";
      }
      webText += "</td></tr>";
      file.close();
//      request->sendChunked( webText );
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }

  webText += "</tbody></table></div>"; // remove + if request->send is uncommented
  webText += getHTMLTFootText( numPic );
  webText += "</div></div></div></div></body></html>";
  request->send( 200, "text/html", webText );

  unsigned long atEnd = millis();
  DBG_OUTPUT_PORT.printf( "Time in listDirectory: %lu milisec\n", atEnd - atStart );
  DBG_OUTPUT_PORT.printf( "Heap after listDirectory: %u\n", ESP.getFreeHeap() );

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

void asyncHandleScan( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
  }

//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
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
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";
  request->send( 200, "application/json", json );
  json = String();

}

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  String webText = getHTMLRootText();
  request->send( 200, "text/html", webText );

}

void asyncHandleStatistics( AsyncWebServerRequest *request ) {

  String webText = getHTMLStatisticsText();
  request->send( 200, "text/html", webText );

}

void asyncHandleSetup( AsyncWebServerRequest *request ) {

  if( request->authenticate( http_username, http_password ) ) {
    String webText = getHTMLSetupText();
    request->send( 200, "text/html", webText );
  } else {
    request->send( 200, "text/plain", "Not Authorized!" );
  }

}

void asyncHandleLogin( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) )
    return request->requestAuthentication();
  request->send( 200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; URL=/setup'></head><body>Login Success!</body></html>" );

}

void asyncHandleInput( AsyncWebServerRequest *request ) {

  String webText;

  DBG_OUTPUT_PORT.print( "Got arguments - " );

  if( request->hasParam( "onoffswitch" ) ) {
    AsyncWebParameter* arg1 = request->getParam( "onoffswitch" );
    if( arg1->value() == "on" )
      flashEnable = true;
    else
      flashEnable = false;
    DBG_OUTPUT_PORT.print( String( arg1->value() ) );
  }
  DBG_OUTPUT_PORT.print( " - " );

  if( request->hasParam( "picSize" ) ) {
    AsyncWebParameter* arg2 = request->getParam( "picSize" );
    fnSetFrameSize( arg2->value() );
    DBG_OUTPUT_PORT.print( String( arg2->value() ) );
  }
  DBG_OUTPUT_PORT.print( " - " );

  if( request->hasParam( "timePeriod" ) ) {
    AsyncWebParameter* arg3 = request->getParam( "timePeriod" );
    waitTime = arg3->value().toInt(); // TODO - foolproof this
    DBG_OUTPUT_PORT.print( String( arg3->value() ) );
  }
  DBG_OUTPUT_PORT.println( " - !" );

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'>";
  webText += "</head><body>Set!</body></html>";
  request->send( 200, "text/html", webText );

}

void asyncHandlePictures( AsyncWebServerRequest *request ) {

  File picDir;

  picDir = SD_MMC.open( "/ai-cam" );
  // listDirectoryAsString( picDir );
  listDirectory( picDir, request );
  picDir.close();

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
  } else {
    AsyncWebParameter* argDelete = request->getParam( "FILENAME" );
    String fileName = argDelete->value();
    String webText = "About to DELETE - " + String( fileName );
    DBG_OUTPUT_PORT.print( "About to DELETE - " );
    DBG_OUTPUT_PORT.println( fileName );
    request->send( 200, "text/plain", webText );
//    deleteFile( SD_MMC, fileName );
  }

}

void asyncHandleStartAP( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
  } else {
    WiFi.softAP( "MozzAICam" );
    IPAddress IP = WiFi.softAPIP();
    DBG_OUTPUT_PORT.print( "AP IP address: " );
    DBG_OUTPUT_PORT.println( IP );
    request->send( 200, "text/plain", "AP Started!" );
  }

}

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

  DBG_OUTPUT_PORT.print( "Basename - " );
  DBG_OUTPUT_PORT.println( fileName );
  DBG_OUTPUT_PORT.println( webText );

  bool fileSPIFFS = false;
  if( fileName.endsWith( ".css" ) ) {
    dataType = "text/css";
    fileSPIFFS = true;
  } else if( fileName.endsWith( ".js" ) ) {
    dataType = "aplication/javascript";
    fileSPIFFS = true;
  }
  if( fileSPIFFS ) {
    request->send( SPIFFS, fileName.c_str(), "text/css" );
    return;
  }

  if( loadFromSDCard( request ) ) {
    return;
  }

  webText = "\nNo Handler\r\n" + webText;
  request->send( 404, "text/plain", webText );
  DBG_OUTPUT_PORT.println( webText );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, asyncHandleRoot );
  asyncWebServer.on( "/delete", HTTP_GET, asyncHandleDelete );
  asyncWebServer.on( "/login", HTTP_GET, asyncHandleLogin );
  asyncWebServer.on( "/scan", HTTP_GET, asyncHandleScan );
  asyncWebServer.on( "/set", HTTP_GET, asyncHandleInput );
  asyncWebServer.on( "/setup", HTTP_GET, asyncHandleSetup );
  asyncWebServer.on( "/pictures", HTTP_GET, asyncHandlePictures );
  asyncWebServer.on( "/snaps", HTTP_GET, asyncHandlePictures );
  asyncWebServer.on( "/stats", HTTP_GET, asyncHandleStatistics );
  asyncWebServer.on( "/startap", HTTP_GET, asyncHandleStartAP );
  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}

#endif
