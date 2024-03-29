//
// (c) mozgy
//
// MIT Licence
//

#include <SD_MMC.h>

#include "variables.h"
#include "mywebserver.h"


String getHTMLRootText( void ) {

  String webText;

  webText = "<!doctype html><html><head><title>Mozz Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>";
  webText += "Cam-" + String( CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  webText += "<p><a href=/stats>Statistics</a>";
  webText += "<br><a href=/setup>Setup</a>";
//  webText += "<br><a href=/photo>Photo</a>";
  webText += "</body></html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLStatisticsText( void ) {

  String webText;
  char tmpStr[80];

  webText = "<!doctype html><html><head><title>Mozz Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>";
  webText += "Cam-" + String( CAM_SERIAL ) + "<br>";
  webText += "Software Version " + String( SW_VERSION ) + "<br>";
  fnElapsedStr( elapsedTimeString );
  webText += String( elapsedTimeString );
// Serial.printf( "%s - Startup Time : %d-%02d-%02d %02d:%02d:%02d\n", elapsedTimeString, (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );
#ifdef HAVE_SDCARD
  if ( SDCardOK ) {
    sprintf( tmpStr, "<br>Total space: %lluMB - Used space %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024), SD_MMC.usedBytes() / (1024 * 1024) );
    webText += String( tmpStr );
  }
#endif
  webText += "<br>Time Period " + String( waitTime );
  webText += "</body></html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLSetupText( void ) {

  String webText;

  webText = "<!doctype html><html><head><title>Mozz Cam</title><link rel='stylesheet' type='text/css' href='onoffswitch.css'></head>";
  webText += "<body>Camera Setup<form action='/set'>";
  webText += "<table><tr><td>Flash</td>";
  webText += "<td><div class='flashswitch'><input type='checkbox' name='flashswitch' value='flashOn' class='flashswitch-checkbox' id='flashswitch'";
//  if( flashEnabled )
//    webText += " checked";
  webText += ">";
  webText += "<label class='flashswitch-label' for='flashswitch'><span class='flashswitch-inner'></span><span class='flashswitch-switch'></span></label>";
  webText += "</div></td></tr>";
  webText += "</table>";
  webText += "<p>Picture Size - <select name='picSize'>";
  webText += " <option value='FRAMESIZE_QVGA'>320x240</option>";
  webText += " <option value='FRAMESIZE_VGA'>640x480</option>";
  webText += " <option value='FRAMESIZE_SVGA' selected>800x600</option>";
  webText += " <option value='FRAMESIZE_XGA'>1024x768</option>";
  webText += " <option value='FRAMESIZE_SXGA'>1280x1024</option>";
  webText += " <option value='FRAMESIZE_UXGA'>1600x1200</option>";
  webText += " <option value='FRAMESIZE_QXGA'>2048x1536</option>";
  webText += "</select>";
  webText += "<p><table><tr><td><div>";
  // webText += "<label for='timePeriod'>Time Period - </label>";
  // webText += "<input id='timePeriod' type='number' name='timePeriod' min='10' max='600' step='10' value='" + String( waitTime ) + "'>";
  webText += "<input type='submit' value='Set' class='clickmebutton'>";
  webText += "</div></td></tr><tr><td><a href='/' class='clickmebutton'>Back</a></td></tr></table></form></body></html>";

  return webText; // TODO - make me pwetty !

}

String getHTMLTFootText( int numPic ) {

  String webText;

  webText = "<div class='tableCam-foot'><table><tfoot><tr><th colspan='2'>Number of entries - " + String( numPic ) + "</th></tr>";
  webText += "<tr><th colspan='2'>Back to <a href='/'>TOP</a></th></tr></tfoot></table></div>";

  return webText;

}

bool loadFromSDCard( AsyncWebServerRequest *request ) {

  String dataType;
  String webText;
  String path = request->url();
  int lastSlash = path.lastIndexOf( '/' );
  String baseName = path.substring( lastSlash + 1, path.length() );
  String fileName = path.c_str();

// SDCard load filename - google.com:443
// SDCard load filename - www.sneakersnstuff.com:443
  Serial.printf( "SDCard load - %s, %s, %s\n", path.c_str(), fileName.c_str(), baseName.c_str() );

  File dataFile = SD_MMC.open( fileName );

  if( !dataFile ) {
    return false;
  }

  if( dataFile.isDirectory() ) {
    // webText = listDirectoryAsJSON( dataFile );
    // request->send( 200, "application/json", webText );
    listDirectory( fileName, request );
    dataFile.close();
    return true;
  }
  if( path.endsWith( ".jpg" ) ) {
    dataType = "image/jpeg";
    // request->send( SD_MMC, path.c_str(), String(), true ); // new window - download
    request->send( SD_MMC, fileName, dataType );
    dataFile.close();
    return true;
  }

  return false;

}

void listDirectory( String path, AsyncWebServerRequest *request ) {

  int numPhoto = 0;
  String linkName;
  String hrefName;
  String fileName;
  String webText;
  unsigned long atStart = millis();

  if( HIDE_ROOT_DIR ) {
    String tmpPath = path;
    path = "/mozz-cam" + tmpPath;
  }
  Serial.printf( "listDirectory - %s\n", path.c_str() );

  File linkNameFP = SD_MMC.open( path.c_str() );
  if( !linkNameFP ) {
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'></head><body>Cam Dir *NOT* Initialized!</body></html>" );
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "");
  response->addHeader("Content-Length", "CONTENT_LENGTH_UNKNOWN");
  webText = "<!doctype html><html><head><title>Mozz Cam</title>";
  webText += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'></head>";
  webText += "<body><div class='limiter'><div class='container-tableCam'><div class='wrap-tableCam'>";
  webText += "<div class='tableCam'><div class='tableCam-body'><table><tbody>";
//  request->sendChunked( webText );

  if( linkNameFP.isDirectory() ) {
    File file = linkNameFP.openNextFile();
    while( file ) {
      // ToDo - hide SDCard cam subdir ie '/mozz-cam'
      fileName = String( file.name() );
      hrefName = path + "/" + fileName;
      webText += "<tr><td class='co1'><a href='" + hrefName + "'>" + fileName + "</a></td>";
      Serial.print( " - href - " );
      Serial.println( hrefName );
      webText += "<td class='co2'>";
      if( fileName.endsWith( ".jpg" ) ) {
        webText += "<a href='/delete?filename=" + hrefName + "'>X</a>";
      } else {
        webText += "DIR"; // ToDo - add weblink to delete whole dir
      }
      webText += "</td></tr>";
      file.close();
//      request->sendChunked( webText );
      file = linkNameFP.openNextFile();
//      Serial.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPhoto++;
    }
  }

  webText += "</tbody></table></div>"; // remove + if request->send is uncommented
//  webText += getHTMLTFootText( numPhoto );
  webText += "</div></div></div></div></body></html>";
  request->send( 200, "text/html", webText );

  linkNameFP.close();

  unsigned long atEnd = millis();
  Serial.printf( "Time in listDirectory: %lu milisec\n", atEnd - atStart );
//  Serial.printf( "Heap after listDirectory: %u\n", ESP.getFreeHeap() );

}

/*
String listDirectoryAsString( File path ) {

  String linkName;
  String webText;
  int numPic = 0;

  webText = "<!doctype html><html><head><title>Cam " + String( AI_CAM_SERIAL ) + "</title>";
  webText += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  webText += "<link rel='stylesheet' type='text/css' href='mozz.css'></head>";
  webText += "<body><div class='limiter'><div class='container-tableCam'><div class='wrap-tableCam'>";
  webText += "<div class='tableCam'><div class='tableCam-body'><table><tbody>";
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
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }
  webText += "</tbody></table></div>";
  webText += getHTMLTFootText( numPic );
  webText += "</div></div></div></div></body></html>";
  return webText;

}
  */

String listDirectoryAsJSON( File path ) {

  String linkName;
  int numPic = 0;
  String strJSON = "[";
//  listjson_t table;

  if( path.isDirectory() ) {
    File file = path.openNextFile();
    while( file ) {
      linkName = String( file.name() );
      if( strJSON != "[" ) {
        strJSON += ',';
      }
      strJSON += "{\"type\":\"";
      strJSON += ( file.isDirectory() ) ? "dir" : "file";
      strJSON += "\",\"name\":\"";
      strJSON += String( file.name() ).substring(1);
      strJSON += "\"}";
      file.close();
      file = path.openNextFile();
//      DBG_OUTPUT_PORT.printf( "Heap after openNextFile: %u\n", ESP.getFreeHeap() );
      numPic++;
    }
  }
  strJSON += "]";
  return strJSON;

//  table = fnJSONList( path );

}

/*
<!doctype html>
<html>
 <head>
  <title>Cam " + String( AI_CAM_SERIAL ) + "</title>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <link rel='stylesheet' type='text/css' href='mozz.css'>
 </head>
 <body>
  <div class='limiter'>
   <div class='container-tableCam'>
    <div class='wrap-tableCam'>
     <div class='tableCam'>
      <div class='tableCam-body'>
       <table>
        <tbody>

         <tr>
          <td class='co1'><a href='" + linkName + "'>" + linkName + "</a></td>
          <td class='co2'><a href='/delete?FILENAME=" + linkName + "'>X</a></td>
          or
          <td class='co2'>DIR</td>
         </tr>

        </tbody>
       </table>
      </div>
      <div class='tableCam-foot'>
       <table>
        <tfoot>
         <tr>
          <th colspan='2'>Number of entries - " + String( numPic ) + "</th>
         </tr>
        </tfoot>
       </table>
      </div> // tableCam-foot
     </div>  // tableCam
    </div>   // wrap-tableCam
   </div>    // container-tableCam
  </div>     // limiter
 </body>
</html>
 */
