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

  webText = "<!DOCTYPE html><html>";
  webText += "<title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<style>";
  webText += "table, th, td { border: 1px solid black; border-collapse: collapse; }";
  webText += "th, td { padding: 4px }";
  webText += "</style>";
  webText += "<body>";
  webText += "<table style='width:100%'>";
  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText += "<tr>";
      webText += "<td><a href='" + linkName + "'>" + linkName + "</a></td>";
      webText += "<td align='center'>";
      if( linkName.endsWith( ".jpg" ) ) {
        webText += "X";
      } else {
        webText += "DIR";
      }
      webText += "</td>";
      webText += "</tr>";
      file = path.openNextFile();
      numPic++;
    }
  }
  webText += "</table>";
  webText += "Number of entries - " + String( numPic );
  webText += "</body>";
  webText += "</html>";
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

void handleInput( void ) {

  String arg1;
  String arg2;
  String arg3;
  String webText;

  arg1 = webServer.arg( "onoffswitch" );
  arg2 = webServer.arg( "picSize" );
  arg3 = webServer.arg( "timePeriod" );

  Serial.print( "Got arguments - " );
  Serial.print( String( arg1 ) );
  Serial.print( " - " );
  Serial.print( String( arg2 ) );
  Serial.print( " - " );
  Serial.print( String( arg3 ) );
  Serial.println( " - !" );

  if( arg1 == "on" )
    flashEnable = true;
  else
    flashEnable = false;

  waitTime = arg3.toInt();

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>Set!</body></html>";
  webServer.send( 200, "text/html", webText );

}

void handleRoot( void ) {

  String webText;
  char tmpStr[20];

  webText = "<!DOCTYPE html><html>";
  webText += "<head><title>Cam " + String( AI_CAM_SERIAL ) + "</title></head>";
  webText += "<body>";
  webText += "AI-Cam-" + String( AI_CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  ElapsedStr( elapsedTimeString );
  webText += String( elapsedTimeString ) + "<br>";
  sprintf( tmpStr, "Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );
  webText += String( tmpStr );
  webText += "<p><a href=/setup>Setup</a>";
  webText += "<p><a href=/snaps>Pictures</a>";
  webText += "</body>";
  webText += "</html>";

  webServer.send( 200, "text/html", webText ); // TODO - make me pwetty !

}

void handleSettings( void ) {

  String webText;
  char tmpStr[20];

  webText = "<!DOCTYPE html><html>";
  webText += "<head><link rel='stylesheet' type='text/css' href='onoffswitch.css'></head>";
  webText += "<body>Camera Setup<p>";
  webText += "<form action='/set'><div class='onoffswitch'>";
  webText += "<input type='checkbox' name='onoffswitch' class='onoffswitch-checkbox' id='myonoffswitch'";
  if( flashEnable )
    webText += " checked";
  webText += ">";
  webText += "<label class='onoffswitch-label' for='myonoffswitch'>";
  webText += "<span class='onoffswitch-inner'></span><span class='onoffswitch-switch'></span>";
  webText += "</label>";
  webText += "</div>";
  webText += "<p>Picture Size - <select name='picSize'>";
  webText += "  <option value='FRAMESIZE_QVGA'>320x240</option>";
  webText += "  <option value='FRAMESIZE_VGA'>640x480</option>";
  webText += "  <option value='FRAMESIZE_SVGA'>800x600</option>";
  webText += "  <option value='FRAMESIZE_XGA'>1024x768</option>";
  webText += "  <option value='FRAMESIZE_SXGA'>1280x1024</option>";
  webText += "  <option value='FRAMESIZE_UXGA'>1600x1200</option>";
  webText += "</select>";
  webText += "<p><div>";
  webText += "<label for='timePeriod'>Time Period - </label>";
  webText += "<input id='timePeriod' type='number' name='timePeriod' min='10' max='600' step='10' value='";
  webText += String( waitTime );
  webText += "'>";
  webText += "<input type='submit' value='Set'>";
  webText += "</div>";
  webText += "</form>";
  webText += "</body>";
  webText += "</html>";

  webServer.send( 200, "text/html", webText ); // TODO - make me vewy pwetty !

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
// placeholder=\"multiple of 10\"
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

  webServer.on( "/", HTTP_GET, handleRoot );

  webServer.on( "/onoffswitch.css", HTTP_GET, []() {
    File dataFile = SPIFFS.open( "/onoffswitch.css", "r" );
    webServer.streamFile( dataFile, "text/css" );
    dataFile.close();
  });

  webServer.on( "/json", HTTP_GET, handleJSonList );
  webServer.on( "/setup", HTTP_GET, handleSettings );
  webServer.on( "/set", HTTP_GET, handleInput );
  webServer.on( "/snaps", HTTP_GET, handlePictures );

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

// void send(File content, const String& path, const String& contentType=String(), bool download=false, AwsTemplateProcessor callback=nullptr);
//  if( request->hasParam( "download" ) ) dataType = "application/octet-stream";
//  response->addHeader("Content-Encoding", "gzip");

  request->send( dataFile, path.c_str(), dataType );

  dataFile.close();

  return false;
  return true;

}

void asyncHandleInput( AsyncWebServerRequest *request ) {

//  String arg1;
//  String arg2;
  String webText;

//  arg1 = webServer.arg( "onoffswitch" );
//  arg2 = webServer.arg( "timePeriod" );

//  if( arg1 == "on" )
//    flashEnable = true;
//  else
//    flashEnable = false;

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>Set!</body></html>";
  request->send( 200, "text/html", webText );

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
  asyncWebServer.on( "/onoffswitch.css", HTTP_GET, [](AsyncWebServerRequest *request ){
    request->send( SPIFFS, "/onoffswitch.css", "text/css" );
  });
  asyncWebServer.on( "/set", HTTP_GET, asyncHandleInput );
  asyncWebServer.on( "/snaps", HTTP_GET, asyncHandlePictures );
  asyncWebServer.onNotFound( asyncHandleNotFound );

/*
//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
asyncWebServer.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
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
      json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";
  request->send(200, "application/json", json);
  json = String();
});
 */

  asyncWebServer.begin();

}
