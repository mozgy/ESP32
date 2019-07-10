//
// (c) mozgy
//
// MIT Licence
//

/// WebServer Definitions

String listDirectory( File path ) {

  String linkName;
  String webText;
  int numPic = 0;

  webText = "<!DOCTYPE html><html>\n";
  webText += "<title>Cam " + String( AI_CAM_SERIAL ) + "</title>\n";
  webText += "<body>";
  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText += "<a href=\"" + linkName + "\">" + linkName + "</a><br>";
      file = path.openNextFile();
      numPic++;
    }
    webText += "Number of entries - " + String( numPic ) + "<br>";
    webText += "</body>";
    webText += "</html>";
  }
  return webText;

}

bool loadFromSDCard( String path ) {

  String dataType = "text/plain";
  String webText;

  File dataFile = SD_MMC.open( path.c_str() );

  if( !dataFile ) {
    return false;
  }

// NO index.html handling for '/' - maybe TODO later
  if( dataFile.isDirectory() ) {
    webText = listDirectory( dataFile );
    webServer.send( 200, "text/html", webText );
    dataFile.close();
    return true;
  }
  if( path.endsWith( ".jpg" ) ) {
    dataType = "image/jpeg";
  }

  if( webServer.hasArg( "download" ) ) {
    dataType = "application/octet-stream";
  }

  if( webServer.streamFile( dataFile, dataType ) != dataFile.size() ) {
    Serial.println( "Sent less data than expected!" );
  }

  dataFile.close();
  return true;

}

void handleRoot( void ) {

  String webText;
  char tmpStr[20];

  webText = "<!DOCTYPE html><html>\n";
  webText += "<title>Cam " + String( AI_CAM_SERIAL ) + "</title>\n";
  webText += "<body>";
  webText += "AI-Cam-" + String( AI_CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  ElapsedStr( elapsedTimeString );
  webText += String( elapsedTimeString ) + "<br>";
  sprintf( tmpStr, "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );
  webText += String( tmpStr ) + "<p>";
  webText += "<a href=/snaps>Pictures</a><br>";
  webText += "</body>";
  webText += "</html>";

  webServer.send( 200, "text/html", webText ); // TODO - make me pwetty !

}

void handleJSonList( void ) {

  File picDir = SD_MMC.open( "/ai-cam" );
  String webText = "[";

  if( picDir.isDirectory() ){
    File file = picDir.openNextFile();
    while( file ){
      if( webText != "[" ) {
        webText += ',';
      }
      webText += "{\"type\":\"";
      webText += ( file.isDirectory() ) ? "dir" : "file";
      webText += "\",\"name\":\"";
      webText += String( file.name() ).substring(1);
      webText += "\"}";
      file = picDir.openNextFile();
    }
  }
  webText += "]";
  webServer.send( 200, "text/json", webText );
  picDir.close();

}

void handlePictures( void ) {

  File picDir;
  String webText;

  picDir = SD_MMC.open( "/ai-cam" );
  webText = listDirectory( picDir );
  webServer.send( 200, "text/html", webText );
  picDir.close();

}

void handleNotFound( void ) {

  if( loadFromSDCard( webServer.uri() ) ) {
    return;
  }

  String webText = "File Not Found\n\n";
  webText += "URI: ";
  webText += webServer.uri();
  webText += "\nMethod: ";
  webText += ( webServer.method() == HTTP_GET ) ? "GET" : "POST";
  webText += "\nArguments: ";
  webText += webServer.args();
  webText += "\n";
  for( uint8_t i = 0; i < webServer.args(); i++ ) {
    webText += " " + webServer.argName( i ) + ": " + webServer.arg( i ) + "\n";
  }
  webServer.send( 404, "text/plain", webText );

}

void initWebServer( void ) {

  webServer.on( "/", handleRoot );

  webServer.on( "/json", handleJSonList );

  webServer.on( "/snaps", handlePictures );

  webServer.onNotFound( handleNotFound );
/*
  webServer.onNotFound([]() {
    if ( !handleFileRead( server.uri() ) )
      server.send( 404, "text/plain", "404: Not Found" );
  });
 */

  webServer.begin();

/* can this work this way
  webServer
    .onNotFound
    .on
    .on
    ...
 */

}

/// AsyncWebServer definitions

bool loadFromSDCard( AsyncWebServerRequest *request ) {

  String dataType;
  String webText;
  String path = request->url();

  File dataFile = SD_MMC.open( path.c_str() );

  if( !dataFile ) {
    return false;
  }

// NO index.html handling for '/' - maybe TODO later
  if( dataFile.isDirectory() ) {
    webText = listDirectory( dataFile );
    request->send( 200, "text/html", webText );
    dataFile.close();
    return true;
  }
  if( path.endsWith( ".jpg" ) ) {
    dataType = "image/jpeg";
  }

  if( request->hasParam( "download" ) ) dataType = "application/octet-stream";

  if( webServer.streamFile( dataFile, dataType ) != dataFile.size() ) {
    Serial.println( "Sent less data than expected!" );
  }

  dataFile.close();

  return false;
  // return true;

}

void asyncHandlePictures( AsyncWebServerRequest *request ) {

  File picDir;
  String webText;

  picDir = SD_MMC.open( "/ai-cam" );
  webText = listDirectory( picDir );
  request->send( 200, "text/html", webText );
  picDir.close();

}

void asyncHandleNotFound( AsyncWebServerRequest *request ) {

  String path = request->url();

  // TODO - no index.html here, just plain dir listing
  if( loadFromSDCard( request ) ) {
    return;
  }

  String webText = "\nNo Handler\r\n";
  webText += "URI: ";
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
  request->send( 404, "text/plain", webText );
  Serial.println( webText );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, [](AsyncWebServerRequest *request ){
    request->send(200, "text/plain", "Hello, world");
  });
  asyncWebServer.on( "/snaps", HTTP_GET, asyncHandlePictures );
  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}
