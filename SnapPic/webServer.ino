//
// (c) mozgy
//
// MIT Licence
//

/// WebServer Definitions

void listDirectory( File path ) {

  String linkName;
  String webText;
  int numPic = 0;
  unsigned long atStart = millis();

  webServer.setContentLength( CONTENT_LENGTH_UNKNOWN );
  webServer.send( 200, "text/html", "" );
  webText = "<!DOCTYPE html><html><head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'></head>";
  webText += "<body><div class='limiter'><div class='container-tableCam'><div class='wrap-tableCam'>";
  webText += "<div class='tableCam'><div class='tableCam-body'><table><tbody>";
  webServer.sendContent( webText );

  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      webText = "<tr><td class='co1'><a href='" + linkName + "'>" + linkName + "</a></td>";
      webText += "<td class='co2'>";
      if( linkName.endsWith( ".jpg" ) ) {
        webText += "<a href='/delete?FILENAME=" + linkName + "'>X</a>";
      } else {
        webText += "DIR";
      }
      webText += "</td></tr>";
      file.close();
      webServer.sendContent( webText );
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }

  webText = "</tbody></table></div>";
  webText += getHTMLTFootText( numPic );
  webText += "</div></div></div></div></body></html>";
  webServer.sendContent( webText );

  unsigned long atEnd = millis();
  DBG_OUTPUT_PORT.printf( "Time in listDirectory: %lu milisec\n", atEnd - atStart );
  DBG_OUTPUT_PORT.printf( "Heap after listDirectory: %u\n", ESP.getFreeHeap() );

}

bool loadFromSDCard( String path ) {

  String dataType = "text/plain";
  String webText;

  File dataFile = SD_MMC.open( path.c_str() );

  if( !dataFile ) {
    return false;
  }

  if( dataFile.isDirectory() ) {
    listDirectory( dataFile );
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

  webText = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; URL=/'>";
  webText += "</head><body>Set!</body></html>";
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

void handleStatistics( void ) {

  String webText = getHTMLStatisticsText();
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
      file.close();
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
  listDirectory( picDir );
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
  webServer.on( "/stats", HTTP_GET, handleStatistics );
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
