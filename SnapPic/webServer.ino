//
// (c) mozgy
//
// MIT Licence
//

String fnOptionVGA( char *str1, char *str2 ) {

  String webText;

  webText = "  <option value='";
  webText += String( str1 ) + "'";
  if( String( foo[picSnapSize] ) == String ( str1 ) ) {
    webText += "selected";
  }
  webText += ">" + String( str2 ) + "</option>";

  return webText;

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

String getHTMLRootText( void ) {

  String webText;
  char tmpStr[20];

  webText = "<!DOCTYPE html><html>";
  webText += "<head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'>";
  webText += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<style>body { font-family: 'Comic Sans MS', cursive, sans-serif; }</style>";
  webText += "</head><body>";
  webText += "AI-Cam-" + String( AI_CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  ElapsedStr( elapsedTimeString );
  webText += String( elapsedTimeString ) + "<br>";
  sprintf( tmpStr, "Used space %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024) );
  webText += String( tmpStr );
  webText += "<br>Time Period " + String( waitTime );
//  webText += "<p><a href=/setup>Setup</a>";
  webText += "<p><a href=/snaps>Pictures</a>";
  webText += "</body>";
  webText += "</html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLSetupText( void ) {

  String webText;

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
  webText += "  <option value='FRAMESIZE_XGA' selected>1024x768</option>";
  webText += "  <option value='FRAMESIZE_SXGA'>1280x1024</option>";
  webText += "  <option value='FRAMESIZE_UXGA'>1600x1200</option>";
  webText += "  <option value='FRAMESIZE_QXGA'>2048x1536</option>";
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

  return webText; // TODO - make me pwetty !
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

String listDirectory( File path ) {

  String linkName;
  String webText;
  int numPic = 0;

  webText = "<!DOCTYPE html><html><head>";
  webText += "<title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'>";
  webText += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'>";
  webText += "</head>";
  webText += "<body>";
  webText += "<div class='limiter'>";
  webText += "<div class='container-tableCam'>";
  webText += "<div class='wrap-tableCam'>";
  webText += "<div class='tableCam'>";
  webText += "<div class='tableCam-body'>";
  webText += "<table><tbody>";
  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText += "<tr>";
      webText += "<td class='column1'><a href='" + linkName + "'>" + linkName + "</a></td>";
      webText += "<td class='column2'>";
      if( linkName.endsWith( ".jpg" ) ) {
        webText += "<a href='/delete?FILENAME=" + linkName + "'>X</a>";
      } else {
        webText += "DIR";
      }
      webText += "</td>";
      webText += "</tr>";
      file = path.openNextFile();
      numPic++;
    }
  }
  webText += "</tbody></table></div>";
  webText += "<div class='tableCam-foot'><table><tfoot>";
  webText += "<tr><th colspan='2'>Number of entries - " + String( numPic ) + "</th></tr>";
  webText += "</tfoot></table>";
  webText += "</div>"; // tableCam-foot
  webText += "</div>"; // tableCam
  webText += "</div>"; // wrap-tableCam
  webText += "</div>"; // container-tableCam
  webText += "</div>"; // limiter
  webText += "</body>";
  webText += "</html>";
  return webText;

}

/// WebServer Definitions

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
    DBG_OUTPUT_PORT.println( "Sent less data than expected!" );
  }

  dataFile.close();
  return true;

// FIXME - put SD_MMC stuff in separate file !?
//  listDir(SD_MMC, "/", 0);
//  removeDir(SD_MMC, "/mydir");
//  createDir(SD_MMC, "/mydir");
//  deleteFile(SD_MMC, "/hello.txt");
//  writeFile(SD_MMC, "/hello.txt", "Hello ");
//  appendFile(SD_MMC, "/hello.txt", "World!\n");
//  listDir(SD_MMC, "/", 0);

}

void handleInput( void ) {

  String arg1;
  String arg2;
  String arg3;
  String webText;

  arg1 = webServer.arg( "onoffswitch" );
  arg2 = webServer.arg( "picSize" );
  arg3 = webServer.arg( "timePeriod" );

  if( arg1 == "on" )
    flashEnable = true;
  else
    flashEnable = false;

  fnSetFrameSize( arg2 );

  waitTime = arg3.toInt(); // TODO - foolproof this

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>Set!</body></html>";
  webServer.send( 200, "text/html", webText );

/*
  DBG_OUTPUT_PORT.print( "Got arguments - " );
  DBG_OUTPUT_PORT.print( String( arg1 ) );
  DBG_OUTPUT_PORT.print( " - " );
  DBG_OUTPUT_PORT.print( String( arg2 ) );
  DBG_OUTPUT_PORT.print( " - " );
  DBG_OUTPUT_PORT.print( String( arg3 ) );
  DBG_OUTPUT_PORT.println( " - !" );
 */

}

void handleRoot( void ) {

  String webText = getHTMLRootText();
  webServer.send( 200, "text/html", webText );

}

void handleSetup( void ) {

  if( webServer.authenticate( http_username, http_password ) ) {
    String webText = getHTMLSetupText();
    webServer.send( 200, "text/html", webText );
  } else {
    webServer.send( 200, "text/plain", "Not Authorized!" );
  }

}

void handleLogin( void ) {

  if( !webServer.authenticate( http_username, http_password ) )
    return webServer.requestAuthentication();
  webServer.send( 200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; URL=/setup'></head><body>Login Success!</body></html>" );

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
  webServer.send( 200, "application/json", webText );
  picDir.close();

}

void handleDeleteSDCardFile( void ) {

  if( !webServer.authenticate( http_username, http_password ) ) {
    webServer.send( 200, "text/plain", "Not Authorized!" );
    return;
  }

  String webText;

  if( webServer.hasArg( "FILENAME" ) ) {
    String fileName = webServer.arg( "FILENAME" );
    webText = "About to DELETE - " + String( fileName );
    DBG_OUTPUT_PORT.print( "About to DELETE - " );
    DBG_OUTPUT_PORT.println( fileName );
  } else {
    webText = "Nothing To Do !";
    DBG_OUTPUT_PORT.println( "Nothing To Do !" );
  }

  webServer.send( 200, "text/plain", webText );

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

  String path = webServer.uri();
  String dataType = "text/plain";
  String webText;

  int lastSlash = path.lastIndexOf( '/' );
  String fileName = path.substring( lastSlash, path.length() );

  webText = "URI: ";
  webText += webServer.uri();
  webText += "\nMethod: ";
  webText += ( webServer.method() == HTTP_GET ) ? "GET" : "POST";
  webText += "\nArguments: ";
  webText += webServer.args();
  webText += "\n";
  for( uint8_t i = 0; i < webServer.args(); i++ ) {
    webText += " " + webServer.argName( i ) + ": " + webServer.arg( i ) + "\n";
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
    File dataFile = SPIFFS.open( fileName.c_str(), "r" );
    webServer.streamFile( dataFile, dataType );
    dataFile.close();
    return;
  }

  if( loadFromSDCard( path ) ) {
    return;
  }

  webText = "File Not Found\n\n" + webText;
  webServer.send( 404, "text/plain", webText );

}

void initWebServer( void ) {

  webServer.on( "/", HTTP_GET, handleRoot );
  webServer.on( "/delete", HTTP_GET, handleDeleteSDCardFile );
//  webServer.on( "/json", HTTP_GET, handleJSonList );
  webServer.on( "/login", HTTP_GET, handleLogin );
  webServer.on( "/set", HTTP_GET, handleInput );
  webServer.on( "/setup", HTTP_GET, handleSetup );
  webServer.on( "/snaps", HTTP_GET, handlePictures );

// handleNotFound serves all *.js and *.css files from SPIFFS
  webServer.onNotFound( handleNotFound );

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

// NO index.html handling for '/' - maybe TODO later - HTML stuff is on SPIFFS
  if( dataFile.isDirectory() ) {
    webText = listDirectory( dataFile );
    request->send( 200, "text/html", webText );
    dataFile.close();
    return true;
  }
  if( path.endsWith( ".jpg" ) ) {
    dataType = "image/jpeg";
    // request->send( SD_MMC, path.c_str(), String(), true );
    request->send( SD_MMC, path.c_str(), dataType );
    // request->send(SPIFFS, "/test.jpg", "image/jpeg");
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
  request->send(200, "application/json", json);
  json = String();

}

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  String webText = getHTMLRootText();
  request->send( 200, "text/html", webText );

}

void asyncHandleSetup( AsyncWebServerRequest *request ) {

  if( request->authenticate( http_username, http_password ) ) {
    String webText = getHTMLSetupText();
    request->send( 200, "text/html", webText );
  } else {
    // request->send( 200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; URL=/setup'></head><body>Login Success!</body></html>" );
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
  AsyncWebParameter* arg1;
  AsyncWebParameter* arg2;
  AsyncWebParameter* arg3;

  if( request->hasParam( "onoffswitch" ) )
    arg1 = request->getParam( "onoffswitch" );

  if( arg1->value() == "on" )
    flashEnable = true;
  else
    flashEnable = false;

  if( request->hasParam( "picSize" ) ) {
    arg2 = request->getParam( "picSize" );
    fnSetFrameSize( arg2->value() );
  }

  if( request->hasParam( "timePeriod" ) ) {
    arg3 = request->getParam( "timePeriod" );
    waitTime = arg3->value().toInt(); // TODO - foolproof this
  }

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>Set!</body></html>";
  request->send( 200, "text/html", webText );

/*
  DBG_OUTPUT_PORT.print( "Got arguments - " );
  DBG_OUTPUT_PORT.print( String( arg1->value() ) );
  DBG_OUTPUT_PORT.print( " - " );
  DBG_OUTPUT_PORT.print( String( arg2->value() ) );
  DBG_OUTPUT_PORT.print( " - " );
  DBG_OUTPUT_PORT.print( String( arg3->value() ) );
  DBG_OUTPUT_PORT.println( " - !" );
 */

}

void asyncHandlePictures( AsyncWebServerRequest *request ) {

  File picDir;
  String webText;

  picDir = SD_MMC.open( "/ai-cam" );
  webText = listDirectory( picDir );
  request->send( 200, "text/html", webText );
  picDir.close();

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/plain", "Not Authorized!" );
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
  asyncWebServer.on( "/snaps", HTTP_GET, asyncHandlePictures );
  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}

/*

  asyncWebServer.on( "/", HTTP_GET, []( AsyncWebServerRequest *request ){
    String webText = getHTMLRootText();
    request->send( 200, "text/html", webText );
  });

  asyncWebServer.on( "/login", HTTP_GET, []( AsyncWebServerRequest *request ){
    if( !request->authenticate( http_username, http_password ) )
      return request->requestAuthentication();
    request->send( 200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; URL=/setup'></head><body>Login Success!</body></html>" );
  });

 */
